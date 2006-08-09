/*
 * wrapper class for ffmpeg
 * Copyright (C) 2005-2006 Sulejman Mundzic 
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 */

#include <avcodec.h>
#include <avformat.h>

#include "avHandler.h"

void AVHandler::AVHandler()
  :init(false)
  encodePixmapFmt(PixmapFormat::PIXFMT_BGR32, picture_width, picture_height),
  decodePixmapFmt(PixmapFormat::PIXFMT_BGR32, picture_width, picture_height)
{
  std::cerr<<"AVCodecHandler::AVCodecHandler()\n";
    
  av_pix_id[PixmapFormat::PIXFMT_NONE]     = PIX_FMT_NONE;

  av_pix_id[PixmapFormat::PIXFMT_RGB15_LE] = PIX_FMT_RGB555;
  av_pix_id[PixmapFormat::PIXFMT_RGB16_LE] = PIX_FMT_RGB565;
  av_pix_id[PixmapFormat::PIXFMT_RGB15_BE] = PIX_FMT_RGB555;
  av_pix_id[PixmapFormat::PIXFMT_RGB16_BE] = PIX_FMT_RGB565;

  av_pix_id[PixmapFormat::PIXFMT_BGR24]    = PIX_FMT_BGR24;
  av_pix_id[PixmapFormat::PIXFMT_BGR32]    = PIX_FMT_RGBA32;
  av_pix_id[PixmapFormat::PIXFMT_RGB24]    = PIX_FMT_RGB24;
  av_pix_id[PixmapFormat::PIXFMT_RGB32]    = PIX_FMT_RGBA32;

  av_pix_id[PixmapFormat::PIXFMT_YUYV]     = PIX_FMT_YUV422;
  av_pix_id[PixmapFormat::PIXFMT_YUV422P]  = PIX_FMT_YUV422P;
  av_pix_id[PixmapFormat::PIXFMT_YUV420P]  = PIX_FMT_YUV420P;
  av_pix_id[PixmapFormat::PIXFMT_UYVY]     = PIX_FMT_UYVY422;

  std::map<PixmapFormat::PixelFormat,enum PixelFormat>::iterator it;
  for(it = av_pix_id.begin(); it != av_pix_id.end(); it++)
    ptt_pix_id[ it->second ] = it->first;
  
}

void AVHandler::~AVHandler()
{
  
}

bool AVHandler::init(BaseCodecConfig& _config)
{
  if (init) {
    std::cerr<<"AVHandler::init: second call to init - discarded\n";
    return (false);
  }

  init = true;

  config = &_config; // config should not be deleted befor end of this object

  // register all codecs available on ffmpeg
  av_register_all();

  // just get an object
  outputFormat = guess_format(NULL, NULL, "video/mpg"); // application/ogg

  outputFormat->video_codec = (CodecID) config->videoCodecID;

  if (!(outputContext = av_alloc_format_context())) {
    std::cerr << "AVHandler::init: ERROR could not allocate format context\n";
    return (false);
  }

  outputContext->oformat = outputFormat;  

  if (outputFormat->video_codec != CODEC_ID_NONE) {

    if (!(outputVideoStream = av_new_stream(outputContext,0))) {
      std::cerr << "AVHandler::init: ERROR could not allocate new stream\n";
      return (false);
    }
      
    encoderCodecContext = outputVideoStream->codec;
    
    encoderCodecContext->codec_id   = outputFormat->video_codec;
    encoderCodecContext->codec_type = CODEC_TYPE_VIDEO;
    encoderCodecContext->bit_rate   = config->videoCodecBitrate ; //128000; //64000;
    encoderCodecContext->pix_fmt    = PIX_FMT_YUV420P;
    encoderCodecContext->width      = config->encodeFormat.width();         // 384;  
    encoderCodecContext->height     = config->encodeFormat.height();        // 288;
    encoderCodecContext->time_base  = (AVRational){1,12};
    encoderCodecContext->gop_size   = 12; // emit one intra frame every second
  }

  if (av_set_parameters(outputContext, NULL) < 0) {
    std::cerr << "AVHandler::init: ERROR could not set parameters\n";
    return (false);
  }

  dump_format(outputContext, 0, NULL, 1);

  encoderCodec = avcodec_find_encoder(encoderCodecContext->codec_id);
  if (avcodec_open(encoderCodecContext, encoderCodec) < 0) {
    std::cerr << "AVHandler::init: ERROR could not open encoder\n";
    return (false);
  }

  // set buffer size for picture
  encodeOutbufSize = EncoderBufferSize;
  encodeOutbuf = (uint8_t*)malloc(encodeOutbufSize);

  encodePictureBuffer = avcodec_alloc_frame();

  yuvDataPtr = (uint8_t*)malloc(avpicture_get_size(PIX_FMT_YUV420P,
						   encoderCodecContext->width,
						   encoderCodecContext->height));

  // the only difference between streams and files is, that AVOutputFormat* AVOF->name 
  // should be xyz_stream for a stream and anything else for a file



}

VideoFrame* AVHandler::convertVideoFrame(const VideoFrame* in_frame) const
{
  std::cerr<<"AVHandler::convert_video_frame()\n";
  
  VideoFrame* out_frame = new VideoFrame();
  out_frame->fmt.setFormat(decodePixmapFmt.id(),
			   in_frame->fmt.width(),
			   in_frame->fmt.height());
  
  out_frame->data_ptr = (unsigned char*)malloc(out_frame->fmt.size());
  if (out_frame->data_ptr == NULL) return NULL;
  
  avpicture_fill((AVPicture *)out_conv_frame,
		 out_frame->data_ptr,
		 av_pix_id[out_frame->fmt.id()],
		 out_frame->fmt.width(),out_frame->fmt.height());

  
  avpicture_fill((AVPicture *)in_conv_frame,
		 in_frame->data_ptr,
		 av_pix_id[in_frame->fmt.id()],
		 in_frame->fmt.width(),in_frame->fmt.height());
  
  
  img_convert((AVPicture *)out_conv_frame,
	      av_pix_id[out_frame->fmt.id()], 
	      (AVPicture *)in_conv_frame, 
	      av_pix_id[in_frame->fmt.id()],
	      in_frame->fmt.width(),in_frame->fmt.height());
  
  return out_frame;
}




/* helper routines */

AVFrame* AVHandler::alloc_picture(int pix_fmt, int width, int height)
{
    fprintf(stderr,"AVHandler::alloc_picture()\n");
    AVFrame *picture;
    uint8_t *picture_buf;
    int     size;
    
    picture = avcodec_alloc_frame();
    if (!picture)
        return NULL;
    size = avpicture_get_size(pix_fmt, width, height);
    picture_buf = (uint8_t*)malloc(size);
    if (!picture_buf) {
        av_free(picture);
        return NULL;
    }
    avpicture_fill((AVPicture *)picture, picture_buf, 
                   pix_fmt, width, height);
    return picture;
}

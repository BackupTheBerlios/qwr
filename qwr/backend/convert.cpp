/*
 * conversion between different pixmap formats
 * Copyright (C) 2005-2006 Sulejman Mundzic, Joern Seger 
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

#include "convert.h"
#include "avcodec.h"
#include "avformat.h"

Convert::Convert()
{
    av_register_all();

  //  std::cerr<< "Convert::Convert\n";
  in_conv_frame  = avcodec_alloc_frame();
  out_conv_frame = avcodec_alloc_frame();
  if (!in_conv_frame || !out_conv_frame)
    {
      fprintf(stderr, "could not allocate conversions memory \n");
      exit(1);
    }
  
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
    {
      ptt_pix_id[ it->second ] = it->first;
    }
  fprintf(stderr, "av_pix_id.size() = % d\n", av_pix_id.size());

}

Convert::~Convert()
{
  av_free(in_conv_frame);
  av_free(out_conv_frame);
}

VideoFrame* Convert::convert(VideoFrame* in_frame, PixmapFormat::PixelFormat ID)
{
  //  fprintf(stderr,"AVCodecHandler::convert_video_frame()\n");
  
  VideoFrame* out_frame = new VideoFrame();
  out_frame->fmt.setFormat(ID,
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

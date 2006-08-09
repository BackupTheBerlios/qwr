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

#include"avCodecHandler.h"

#include "avcodec.h"
void pgm_save(unsigned char *buf,int wrap, int xsize,int ysize,char *filename);

std::map<PixmapFormat::PixelFormat,enum PixelFormat> AVCodecHandler::av_pix_id;
std::map<enum PixelFormat,PixmapFormat::PixelFormat> AVCodecHandler::ptt_pix_id;

AVCodecHandler::AVCodecHandler()
    : in_file_fp(NULL), encode_file_fp(NULL),
      picture_width(320), picture_height(240),
      encode_fmt(PixmapFormat::PIXFMT_BGR32, picture_width, picture_height),
      decode_fmt(PixmapFormat::PIXFMT_BGR32, picture_width, picture_height)
{
    fprintf(stderr, "AVCodecHandler::AVCodecHandler()\n");

    /* initialize libavcodec, and register all codecs and formats */
    av_register_all();
    
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


void AVCodecHandler::init(EncoderConfig& config, PixmapFormat::PixelFormat dpy_fmt_id)
{
    encConfig = config;
    init_conv();
    init_encoder(encode_fmt, encConfig);
    //init_encoder(encode_fmt);
    init_decoder(dpy_fmt_id);
    //    init_decoder(PixmapFormat::PIXFMT_NONE);

}

AVCodecHandler::~AVCodecHandler()
{
    fini_conv();
    fini_encoder();
    fini_decoder();
}

FrameSlice* AVCodecHandler::encode_video_frame(VideoFrame* in_frame) const
{
    fprintf(stderr,"AVCodecHandler::encode_video_frame() \n");

    FrameSlice* slice    = NULL;
    int      out_size    = 0;

    if (av_pix_id[in_frame->fmt.id()] != encode_c->pix_fmt)
    {
	// convert to av_pix_id[encode_fmt.id()];
	avpicture_fill((AVPicture *)encode_picture, yuv_data_ptr,
		       encode_c->pix_fmt,
		       encode_c->width, encode_c->height);


	avpicture_fill((AVPicture *)in_conv_frame, in_frame->data_ptr,
		       av_pix_id[in_frame->fmt.id()],
		       in_frame->fmt.width(),in_frame->fmt.height());

	img_convert((AVPicture *)encode_picture, encode_c->pix_fmt,
		    (AVPicture *)in_conv_frame,  av_pix_id[in_frame->fmt.id()],
		    in_frame->fmt.width(), in_frame->fmt.height());
    }
    else
    {
	avpicture_fill((AVPicture *)encode_picture, in_frame->data_ptr,
		   encode_c->pix_fmt, encode_c->width, encode_c->height);
    }

    // encode the image
    out_size = avcodec_encode_video(encode_c, encode_outbuf,
				    encode_outbuf_size, encode_picture);

    fprintf(stderr,"encoding frame %3d (size=%5d)\n",
	    encode_c->frame_number, out_size);


    if (out_size > 0)
    {
	slice = new FrameSlice((char*)encode_outbuf, out_size);
	fprintf(stderr,"out_data_ptr x 2 = %p \n", slice->data());
	fwrite(encode_outbuf, 1, out_size, encode_file_fp);
    }
    return slice;
}


VideoFrame*
AVCodecHandler::decode_video_frame(uint8_t* in_data_ptr, int in_size)
{
    fprintf(stderr,"AVCodecHandler::decode_video_frame_to_native()\n");

    int              size;          /* chunk of the data to be decoded */
    int              got_picture;   /* set to 0 if not got full frame */
    int              len;           /* length of decoded data */

    uint8_t*         decode_inbuf_ptr = decode_inbuf;


    memcpy(decode_inbuf,in_data_ptr,in_size);
    size = in_size;
    fprintf(stderr,"bytes to be decoed red from file: size = %d \n", size);

//    decode_c->pix_fmt = PIX_FMT_RGBA32;

    decode_inbuf_ptr = decode_inbuf;
    while (size > 0) 
    {
	fprintf(stderr,"bytes to be decoed: size = %d \n", size);

	len = avcodec_decode_video(decode_c, decode_picture, &got_picture, 
				   decode_inbuf_ptr, size);

	fprintf(stderr,"decoed bytes: len = %d \n", len);
	
	if (len < 0)
	{
	    fprintf(stderr, "Error while decoding frame %d\n",
		    curr_decode_frame);
	    exit(1);
	    return NULL;
	}
	if (got_picture)
	{
	    fprintf(stderr,"got picture:  saving frame %3d\n",
		    curr_decode_frame);
	    fflush(stdout);

	    /* the picture is allocated by the decoder. no need to free it */
	    fprintf(stderr,"AVCodecHandler:\n"
		    "decode_c->width = %d \n"
		    "decode_c->height = %d \n"
		    "decode_c->pix_fmt = %d \n"
		    "decode_picture->data[0] = %p -> "
		    "decode_picture->linesize[0] = %d \n"
		    "decode_picture->data[1] = %p -> "
		    "decode_picture->linesize[1] = %d \n",
		    decode_c->width,decode_c->height,decode_c->pix_fmt,
		    decode_picture->data[0], decode_picture->linesize[0],
		    decode_picture->data[1], decode_picture->linesize[1]);

	    decode_fmt.setFormat(decode_fmt.id(),
				 decode_c->width,
				 decode_c->height);

	    int out_size_native = decode_fmt.size();
	    
	    
	    fprintf(stderr,"out_size_native = %d \n", out_size_native);
	    uint8_t* out_data_native_ptr = NULL;

	    out_data_native_ptr = (uint8_t*)malloc(out_size_native);
	    if (out_data_native_ptr == NULL)
	    {
		fprintf(stderr,"memory allocation failed \n");
		exit(1);
		return NULL;
	    }

	    out_size_native=avpicture_fill((AVPicture *)decode_native_picture,
			   out_data_native_ptr, 
			   av_pix_id[decode_fmt.id()],
			   decode_c->width, decode_c->height);


	    img_convert((AVPicture *)decode_native_picture,
			av_pix_id[decode_fmt.id()], 
			(AVPicture *)decode_picture, decode_c->pix_fmt,
			decode_c->width, decode_c->height);


	    VideoFrame* decoded_frame = new VideoFrame();
	    decoded_frame->fmt = decode_fmt;
	    decoded_frame->data_ptr = out_data_native_ptr;
	    decoded_frames.push_back(decoded_frame);
	    
	    curr_decode_frame++;
	}
	size = size - len;
	decode_inbuf_ptr = decode_inbuf_ptr + len;
    }
    VideoFrame* out_frame = NULL;
    if (!decoded_frames.empty())
    {
	out_frame = decoded_frames.front();
	decoded_frames.pop_front();
	fprintf(stderr,"decoded_frames.size() = %d \n", decoded_frames.size());
    }
    
    return out_frame;
}



VideoFrame*
AVCodecHandler::convert_video_frame(const VideoFrame* in_frame) const
{
    fprintf(stderr,"AVCodecHandler::convert_video_frame()\n");
  
    VideoFrame* out_frame = new VideoFrame();
    out_frame->fmt.setFormat(decode_fmt.id(),
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

AVFrame* AVCodecHandler::alloc_picture(int pix_fmt, int width, int height)
{
    fprintf(stderr,"AVCodecHandler::alloc_picture()\n");
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

void pgm_save(unsigned char *buf,int wrap, int xsize,int ysize,char *filename) 
{
    fprintf(stderr,"pgm_save()\n");
    FILE *f;
    int i;

    f=fopen(filename,"w");
    fprintf(f,"P5\n%d %d\n%d\n",xsize,ysize,255);
    for(i=0;i<ysize;i++)
        fwrite(buf + i * wrap,1,xsize,f);
    fclose(f);
}



/* initialization routines */

void AVCodecHandler::init_encoder(PixmapFormat& in_fmt, EncoderConfig& config)
{
  fprintf(stderr, "AVCodecHandler::init_encoder()\n");

  // set the encoder config
  encConfig = config;
  encode_fmt = in_fmt;

  av_pix_id[encode_fmt.id()]; // Does this make sense??

  //    if (encode_file_fp != NULL) fini_encoder();

  //    encode_filename  = "encoder_test.mpg";

  //

  fmt = guess_format(NULL, encConfig.filename.c_str(), "video/mpg"); //application/ogg
  if (!fmt)
    {
      fprintf(stderr, "Could not find suitable output format\n");
      exit(1);
    }

    fprintf(stderr, "fmt->video_codec = %d\n", fmt->video_codec);
    //  fmt->video_codec = CODEC_ID_H263P;
    //  fmt->video_codec = CODEC_ID_MPEG4;

    fmt->video_codec = (CodecID)encConfig.videoEncoderID;

    /* allocate the output media context */
    oc = av_alloc_format_context();
    if (!oc)
      {
        fprintf(stderr, "Memory error\n");
        exit(1);
      }

    oc->oformat = fmt;
    snprintf(oc->filename, sizeof(oc->filename), "%s", encConfig.filename.c_str());
    video_st = NULL;
    if (fmt->video_codec != CODEC_ID_NONE)
      {
	//	video_st = add_video_stream(oc, fmt->video_codec);
	video_st = av_new_stream(oc, 0);
	if (!video_st)
	  {
	    fprintf(stderr, "Could not alloc stream\n");
	    exit(1);
	  }
	encode_c = video_st->codec;
	encode_c->codec_id   = fmt->video_codec;
	encode_c->codec_type = CODEC_TYPE_VIDEO;
	encode_c->bit_rate   = encConfig.videoEncoderBitrate; //128000; //64000;
	encode_c->pix_fmt    = PIX_FMT_YUV420P;
	encode_c->width      = encode_fmt.width();         // 384;  
	encode_c->height     = encode_fmt.height();        // 288;
	encode_c->time_base  = (AVRational){1,12};
	encode_c->gop_size   = 10; // emit one intra frame every ten frames
      }
    if (av_set_parameters(oc, NULL) < 0)
      {
        fprintf(stderr, "Invalid output format parameters\n");
        exit(1);
      }

    //dump_format(oc, 0, encode_filename, 1);

    encode_codec = avcodec_find_encoder(encode_c->codec_id);
    avcodec_open(encode_c, encode_codec);

    encode_outbuf_size = 100000;
    encode_outbuf = (uint8_t*)malloc(encode_outbuf_size);
    encode_picture = avcodec_alloc_frame();

    yuv_data_ptr = (uint8_t*)malloc(avpicture_get_size(PIX_FMT_YUV420P,
						       encode_c->width,
						       encode_c->height));


    if (!(fmt->flags & AVFMT_NOFILE))
      {
        if (url_fopen(&oc->pb,encConfig.filename.c_str() , URL_WRONLY) < 0)
	  {
            fprintf(stderr, "Could not open '%s'\n", encConfig.filename.c_str());
            exit(1);
	  }
      }
    av_write_header(oc);

    /*     
    curr_encode_frame = 0;
    encode_c = NULL;

    // output buffer
    encode_outbuf_size = 100000;
    encode_outbuf = (uint8_t*)malloc(encode_outbuf_size);
    if (encode_outbuf == NULL)
      {
	fprintf(stderr, "init_encoder: can't allocate outbuf memory! \n");
	exit(1);
      }

    //  find the mpeg1 video encoder
    //    encode_codec = avcodec_find_encoder(CODEC_ID_MPEG4);
    //    encode_codec = avcodec_find_encoder(CODEC_ID_MPEG1VIDEO);
    encode_codec = avcodec_find_encoder((CodecID)encConfig.videoEncoderID);
    if (!encode_codec)
      {
        fprintf(stderr, "codec not found\n");
        exit(1);
      }

    encode_c = avcodec_alloc_context();
    encode_picture = avcodec_alloc_frame();

    //  put sample parameters
    //    encode_c->bit_rate = 4000000;
    encode_c->bit_rate = 400;
    //  resolution must be a multiple of two
    encode_c->width  = picture_width;  // 320
    encode_c->height = picture_height; // 240
    //  frames per second
    encode_c->time_base= (AVRational){1,12};
    // time base: this is the fundamental unit of time (in seconds) in terms
    // of which frame timestamps are represented. for fixed-fps content,
    // timebase should be 1/framerate and timestamp increments should be
    // identically 1.

    //c->time_base.den = STREAM_FRAME_RATE;  
    //c->time_base.num = 1;


    encode_c->gop_size = 10; // emit one intra frame every ten frames
    //    encode_c->max_b_frames=1;
    encode_c->pix_fmt = PIX_FMT_YUV420P;

    encode_c->rtp_mode = true;
    encode_c->rtp_payload_size = 1024;

    //    encode_c->flags |= CODEC_FLAG_H263P_SLICE_STRUCT;
    //    encode_c->flags |= CODEC_FLAG_H263P_AIC;
    //    encode_c->flags |= CODEC_FLAG_AC_PRED;

    encode_c->slice_count = 2;


    //  open it
    if (avcodec_open(encode_c, encode_codec) < 0)
      {
        fprintf(stderr, "could not open codec\n");
        exit(1);
      }
    //  the codec gives us the frame size, in samples
   

    */
    encode_file_fp = fopen(encConfig.filename.c_str(), "wb");
    if (!encode_file_fp)
      {
        fprintf(stderr, "could not open %s\n", encConfig.filename.c_str());
        exit(1);
      }

    curr_encode_frame = 0;
    fprintf(stderr, "init_encoder() exiting \n");
   
}

void AVCodecHandler::init_encoder(PixmapFormat& in_fmt)
{
  fprintf(stderr, "AVCodecHandler::init_encoder()\n");

  //    encConfig = config;
  encode_fmt = in_fmt;

  av_pix_id[encode_fmt.id()];

  //    if (encode_file_fp != NULL) fini_encoder();

  char* encode_filename  = "none.mpg";

  /////////////////////////////////////////

    fmt = guess_format(NULL, encode_filename, "video/mp4");
    if (!fmt)
      {
        fprintf(stderr, "Could not find suitable output format\n");
        exit(1);
      }
    fprintf(stderr, "fmt->video_codec = %d\n", fmt->video_codec);
    //  fmt->video_codec = CODEC_ID_H263P;
    fmt->video_codec = CODEC_ID_MPEG4;
/// allocate the output media context
    oc = av_alloc_format_context();
    if (!oc)
      {
        fprintf(stderr, "Memory error\n");
        exit(1);
      }
    oc->oformat = fmt;
    snprintf(oc->filename, sizeof(oc->filename), "%s", encode_filename);
    video_st = NULL;
    if (fmt->video_codec != CODEC_ID_NONE)
      {
	//      video_st = add_video_stream(oc, fmt->video_codec);
	video_st = av_new_stream(oc, 0);
	if (!video_st)
	  {
	    fprintf(stderr, "Could not alloc stream\n");
	    exit(1);
	  }
	encode_c = video_st->codec;
	encode_c->codec_id   = fmt->video_codec;
	encode_c->codec_type = CODEC_TYPE_VIDEO;
	encode_c->bit_rate   = 128000; //64000;
	encode_c->pix_fmt    = PIX_FMT_YUV420P;
	encode_c->width      = encode_fmt.width();         // 384;  
	encode_c->height     = encode_fmt.height();        // 288;
	encode_c->time_base  = (AVRational){1,12};
	encode_c->gop_size   = 10; // emit one intra frame every ten frames
      }
    if (av_set_parameters(oc, NULL) < 0)
      {
        fprintf(stderr, "Invalid output format parameters\n");
        exit(1);
      }
    //  dump_format(oc, 0, encode_filename, 1);

    encode_codec = avcodec_find_encoder(encode_c->codec_id);
    avcodec_open(encode_c, encode_codec);

    encode_outbuf_size = 100000;
    encode_outbuf = (uint8_t*)malloc(encode_outbuf_size);
    encode_picture = avcodec_alloc_frame();

    yuv_data_ptr = (uint8_t*)malloc(avpicture_get_size(PIX_FMT_YUV420P,
						       encode_c->width,
						       encode_c->height));

    /*
      if (!(fmt->flags & AVFMT_NOFILE))
      {
      if (url_fopen(&oc->pb, encode_filename, URL_WRONLY) < 0)
      {
      fprintf(stderr, "Could not open '%s'\n", encode_filename);
      exit(1);
      }
      }
      av_write_header(oc);
    */


    /*
      curr_encode_frame = 0;
      encode_c = NULL;

      // output buffer
      encode_outbuf_size = 100000;
      encode_outbuf = (uint8_t*)malloc(encode_outbuf_size);
      if (encode_outbuf == NULL)
      {
      fprintf(stderr, "init_encoder: can't allocate outbuf memory! \n");
      exit(1);
      }

      //  find the mpeg1 video encoder
      //    encode_codec = avcodec_find_encoder(CODEC_ID_MPEG4);
      //    encode_codec = avcodec_find_encoder(CODEC_ID_MPEG1VIDEO);
      encode_codec = avcodec_find_encoder(CODEC_ID_H263P);
      if (!encode_codec)
      {
      fprintf(stderr, "codec not found\n");
      exit(1);
      }

      encode_c = avcodec_alloc_context();
      encode_picture = avcodec_alloc_frame();

      //  put sample parameters
      //    encode_c->bit_rate = 4000000;
      encode_c->bit_rate = 400;
      //  resolution must be a multiple of two
      encode_c->width  = picture_width;  // 320
      encode_c->height = picture_height; // 240
      //  frames per second
      encode_c->time_base= (AVRational){1,15};
      // time base: this is the fundamental unit of time (in seconds) in terms
      // of which frame timestamps are represented. for fixed-fps content,
      // timebase should be 1/framerate and timestamp increments should be
      // identically 1.

      //c->time_base.den = STREAM_FRAME_RATE;  
      //c->time_base.num = 1;


      encode_c->gop_size = 10; // emit one intra frame every ten frames
      //    encode_c->max_b_frames=1;
      encode_c->pix_fmt = PIX_FMT_YUV420P;

      encode_c->rtp_mode = true;
      encode_c->rtp_payload_size = 1024;

      //    encode_c->flags |= CODEC_FLAG_H263P_SLICE_STRUCT;
      //    encode_c->flags |= CODEC_FLAG_H263P_AIC;
      //    encode_c->flags |= CODEC_FLAG_AC_PRED;

      encode_c->slice_count = 2;


      //  open it
      if (avcodec_open(encode_c, encode_codec) < 0)
      {
      fprintf(stderr, "could not open codec\n");
      exit(1);
      }
      //  the codec gives us the frame size, in samples

      encode_file_fp = fopen(encode_filename, "wb");
      if (!encode_file_fp)
      {
      fprintf(stderr, "could not open %s\n", encode_filename);
      exit(1);
      }
    */
    
    curr_encode_frame = 0;
    fprintf(stderr, "init_encoder() exiting \n");
}
   

void AVCodecHandler::fini_encoder()
{
  fprintf(stderr, "AVCodecHandler::fini_encoder()\n");

  if (encode_file_fp == NULL) return;
   
  int out_size;
  do
    {
      fflush(stdout);
        
      out_size = avcodec_encode_video(encode_c, encode_outbuf,
				      encode_outbuf_size, NULL);
      fprintf(stderr, "write frame %3d (size=%5d)\n",
	      curr_encode_frame, out_size);
      fwrite(encode_outbuf, 1, out_size, encode_file_fp);

    } while (out_size);

  // add sequence end code to have a real mpeg file
  encode_outbuf[0] = 0x00;
  encode_outbuf[1] = 0x00;
  encode_outbuf[2] = 0x01;
  encode_outbuf[3] = 0xb7;
  fwrite(encode_outbuf, 1, 4, encode_file_fp);

  fclose(encode_file_fp);
  free(encode_outbuf);

  avcodec_close(encode_c);
  av_free(encode_c);
  av_free(encode_picture);
  free(yuv_data_ptr);

  encode_file_fp = NULL;
}


void AVCodecHandler::init_decoder(PixmapFormat::PixelFormat out_fmt_id)
{
  fprintf(stderr, "AVCodecHandler::init_decoder()\n");
  decode_fmt.setFormat(out_fmt_id, 0, 0);

  outfilename = "/tmp/test%d.pgm";
  //    infilename  = "decoder_test.mpg";

  curr_decode_frame = 0;
  decode_c   = NULL;

  decode_inbuf = (uint8_t*)malloc(INBUF_SIZE + FF_INPUT_BUFFER_PADDING_SIZE);
  if (decode_inbuf == NULL)
    {
      fprintf(stderr, "init_decoder: can't allocate inbuf memory! \n");
      exit(1);
    }
  /* set end of buffer to 0 (this ensures that no overreading */
  /* happens for damaged mpeg streams)                        */
  memset(decode_inbuf + INBUF_SIZE, 0, FF_INPUT_BUFFER_PADDING_SIZE);


  /* find the mpeg1 video decoder */
  //  decode_codec = avcodec_find_decoder(CODEC_ID_MPEG1VIDEO);
  //  decode_codec = avcodec_find_decoder(CODEC_ID_H263);
  decode_codec = avcodec_find_decoder(CODEC_ID_MPEG4);
  if (!decode_codec)
    {
      fprintf(stderr, "decode_codec not found\n");
      exit(1);
    }

  decode_c       = avcodec_alloc_context();
  decode_picture = avcodec_alloc_frame();
  decode_native_picture = avcodec_alloc_frame();

  if(decode_codec->capabilities & CODEC_CAP_TRUNCATED)
    {
      /* we dont send complete frames */
      decode_c->flags|= CODEC_FLAG_TRUNCATED;
      //	decode_c->flags |= CODEC_FLAG_H263P_SLICE_STRUCT;
    }

  /* for some codecs, such as msmpeg4 and mpeg4, width and height
     MUST be initialized there because these info are not available
     in the bitstream */

  /* open it */
  if (avcodec_open(decode_c, decode_codec) < 0) {
    fprintf(stderr, "could not open decode_codec\n");
    exit(1);
  }
    
  /* the codec gives us the frame size, in samples */

  /*
    in_file_fp = fopen(infilename, "rb");
    if (!in_file_fp) {
    fprintf(stderr, "could not open %s\n", infilename);
    exit(1);
    }
  */
    
  curr_decode_frame = 0;
}

void AVCodecHandler::fini_decoder()
{
  fprintf(stderr, "AVCodecHandler::fini_decoder()\n");
  fclose(in_file_fp);

  avcodec_close(decode_c);
  av_free(decode_c);
  av_free(decode_picture);
  av_free(decode_native_picture);
}

void AVCodecHandler::init_conv()
{
  fprintf(stderr, "AVCodecHandler::init_conv()\n");
  in_conv_frame  = avcodec_alloc_frame();
  out_conv_frame = avcodec_alloc_frame();
  if (!in_conv_frame || !out_conv_frame)
    {
      fprintf(stderr, "could not allocate conversions memory \n");
      exit(1);
    }
}

void AVCodecHandler::fini_conv()
{
  fprintf(stderr, "AVCodecHandler::fini_conv()\n");
  av_free(in_conv_frame);
  av_free(out_conv_frame);
}



/*
  void AVCodecHandler::processFrame(struct VideoFrame* inFrame)
  {

  }

  void AVCodecHandler::processFrame(unsigned char* raw_frame,
  struct ng_video_fmt in_fmt)
  {

  }
*/


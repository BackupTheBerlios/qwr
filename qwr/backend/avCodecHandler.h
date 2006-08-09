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

#ifndef AV_CODEC_HANDLER_H
#define AV_CODEC_HANDLER_H


#include <string>
#include <map>
#include <vector>
#include <deque>

#include "avformat.h"
#include "avcodec.h"

#include"pixmapFormat.h"
#include"videoFrame.h"

//! some Information how to encode a stream
class BaseCodecConfig {

 public:

  int           videoCodecID;
  int           videoCodecBitrate;
  int           audioCodecID;
  int           audioCodecBitrate;
  PixmapFormat  encode_fmt;
  PixmapFormat  decode_fmt;

  EncoderConfig()
    : videoCodecID(CODEC_ID_THEORA), 
    videoCodecBitrate(128000),
    audioCodecID(CODEC_ID_MP2),
    audioCodecBitrate(16000),
    encode_fmt(PixmapFormat::PIXFMT_BGR32, 320, 240),
    decode_fmt(PixmapFormat::PIXFMT_BGR32, 320, 240)
      {};

};

/* In theory, encoderID could be everything from avcodec.h (ffmpeg)
However: my choice is at the moment:

CODEC_ID_MPEG1VIDEO
CODEC_ID_MSMPEG4V3
CODEC_ID_MPEG4
CODEC_ID_RV40

*/

class FrameSlice {

 private:
  bool         copy;     //!< is this a copy?
  const char*  data;     //!< frame slice data
  unsigned int length;   //!< frame slice data length

 public:
  FrameSlice(const char* data, const unsigned int length, bool copy = true);
  virtual ~FrameSlice(); 

  const char*  data();
  unsigned int length();
};

inline FrameSlice::FrameSlice(const char* _data, 
			      const unsigend int _length, 
			      bool _copy = true)
{
  copy   = _copy;
  length = _length;
  data = (copy ? (char*)memcpy(new char[length], _data, length) : _data);
}

inline FrameSlice::~FrameSlice()
{ 
  if(copy) 
    delete data; 
} 

inline const char* FrameSlice::data()
{
  return (data);
}

inline unsigned int FrameSlice::length()
{
  return (length);
}

// XXX: Where is this needed ... bit nasty to define it here and not making it 
//      configurable

#define STREAM_DURATION   5.0
//#define STREAM_FRAME_RATE 25 /* 25 images/s */
#define STREAM_FRAME_RATE 25
#define STREAM_NB_FRAMES  ((int)(STREAM_DURATION * STREAM_FRAME_RATE))
#define STREAM_PIX_FMT PIX_FMT_YUV420P /* default pix_fmt */
/* decoder definitions */
//#define INBUF_SIZE 4096
#define INBUF_SIZE 100000

class AVCodecHandler 
{

 private:
  
  //    char                 *filename;
  AVOutputFormat       *fmt;
  AVFormatContext      *oc;
  AVStream             *video_st;
  double               video_pts;
  
  AVFrame              *picture;
  AVFrame              *tmp_picture;
  uint8_t              *video_outbuf;
  int                  frame_count;
  int                  video_outbuf_size;

  /////////////////////////////////////////////////////////////////////////
    
    unsigned int         picture_width;
    unsigned int         picture_height;
    
    /* conversion definitions */
    AVFrame*             in_conv_frame;
    AVFrame*             out_conv_frame;

    /* decoder definitions */
    PixmapFormat         decode_fmt;
    PixelFormat          dpy_fmtid;
    PixmapFormat         dpy_fmt;

    AVCodec*             decode_codec;
    AVCodecContext*      decode_c;
    AVFrame*             decode_picture;
    AVFrame*             decode_native_picture;

    uint8_t              *decode_inbuf;

    int                  curr_decode_frame;

    std::deque<uint8_t*>    frame_queue;
    std::deque<VideoFrame*> decoded_frames;


    /* encoder definitions */
    //    const char           *encode_filename;
    PixmapFormat         encode_fmt;


    AVCodec*             encode_codec;
    AVCodecContext*      encode_c;
    AVFrame*             encode_picture;
    uint8_t*             yuv_data_ptr;

    int                  encode_outbuf_size;
    uint8_t              *encode_outbuf;

    int                  curr_encode_frame;    


    bool                 init;

 protected:
    static std::map<PixmapFormat::PixelFormat, enum PixelFormat> av_pix_id;
    static std::map<enum PixelFormat, PixmapFormat::PixelFormat> ptt_pix_id;

    BaseCodecConfig* Config;

 public:

    AVCodecHandler(CodecType type);
    ~AVCodecHandler();

    virtual void init(BaseCodecConfig& config) = 0;

    void     init(EncoderConfig& confif);
    void     init_conv();
    void     fini_conv();
    void     init_encoder(PixmapFormat& fmt, EncoderConfig& config);
    void     init_encoder(PixmapFormat& fmt);
    void     fini_encoder();
    void     init_decoder(PixmapFormat::PixelFormat);
    void     fini_decoder();

    virtual void process();

    VideoFrame*  convert_video_frame(const VideoFrame* in_frame) const;
    VideoFrame*  decode_video_frame(uint8_t* in_data_ptr, int in_size);
    FrameSlice*  encode_video_frame(VideoFrame* in_frame) const;

    AVFrame* alloc_picture(int pix_fmt, int width, int height);
};


#endif  // AV_CODEC_HANDLER_H

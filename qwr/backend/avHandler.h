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

#ifndef avHandler_h
#define avHandler_h

#include <string>
#include <map>
#include <vector>
#include <deque>

#include "avformat.h"
#include "avcodec.h"

#include "pixmapFormat.h"
#include "videoFrame.h"
#include "frameSlice.h"

//! some Information how to encode a stream
class BaseCodecConfig {

 public:

  unsigned int  pictureWidth;
  unsigned int  pictureHeight;
  int           videoCodecID;
  int           videoCodecBitrate;
  int           audioCodecID;
  int           audioCodecBitrate;
  PixmapFormat  encodeFormat;
  PixmapFormat  decodeFormat;

  EncoderConfig()
    : pictureWidth(320),
    pictureHeight(240),
    videoCodecID(CODEC_ID_THEORA), 
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


//! Base Handler Class for encoding and decoding of video data
class AVHandler {

 protected:

  static const int EncoderBufferSize = 100000;

  BaseCodecConfig* config;

  AVOutputFormat       *outputFormat;
  AVFormatContext      *outputContext;  // only used in f/init - JS
  AVStream             *outputVideoStream; // only used in f/init - JS
  AVCodec              *encodeCodec;
  AVCodecContext       *encodeCodecContext;

  int                   encodeOutbufSize;
  uint8_t              *encodeOutbuf;

  AVFrame*              encodePictureBuffer;

  uint8_t*              yuvDataPtr;

  PixmapFormat         encodePixmapFmt;

  //--------
  
  PixmapFormat         decodePixmapFmt;
  

  double               video_pts;
  
  AVFrame              *picture;
  AVFrame              *tmp_picture;
  uint8_t              *video_outbuf;
  int                  frame_count;
  int                  video_outbuf_size;


  /* encoder definitions */
  int                  curr_encode_frame;    

  static std::map<PixmapFormat::PixelFormat, enum PixelFormat> av_pix_id;
  static std::map<enum PixelFormat, PixmapFormat::PixelFormat> ptt_pix_id;

  VideoFrame* convertVideoFrame(const VideoFrame* in_frame) const;


 public:
  AVHandler();
  virtual AVHandler();

  virtual void init(BaseCodecConfig& config);

};

#endif

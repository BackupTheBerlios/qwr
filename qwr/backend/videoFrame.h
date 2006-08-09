/*
 * Video Frame Class
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

#ifndef VIDEO_FRAME_H
#define VIDEO_FRAME_H


#include"pixmapFormat.h"

struct VideoFrame
{
    VideoFrame() : fmt(PixmapFormat::PIXFMT_YUV420P,0,0) {};
    unsigned char* data()       {return data_ptr;};
    unsigned int   time_stamp() {return msec_ts;};
    unsigned int         seqNo;
    PixmapFormat         fmt;
    unsigned int         msec_ts;
    unsigned char*       data_ptr;

  /*
  VideoFrame* clone()
  {
    VideoFrame* ret = new VideoFrame;
    ret->fmt = fmt;
    ret->seqNo = seqNo;
    ret->msec_ts = msec_ts;
    memcpy(ret->data_ptr = new unsigned char[320*240*3/2], data_ptr,320*240*3/2); // DANGER!!
    return(ret);
  }
*/
};


#endif // VIDEO_FRAME_H

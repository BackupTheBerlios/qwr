/*
 * message for data transfer between display and application
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

#ifndef videoDisplayMsg_h
#define videoDisplayMsg_h

#include "message.h"
#include "pixmapFormat.h"
#include "videoFrame.h"

namespace VideoDisplayMsg {

  enum MsgType {
    type_dpy,
    type_resize,
    type_frame,
    type_dpySize
  };

  class Dpy : public Message {

  public:
    PixmapFormat::PixelFormat      fmt_id;

    Dpy(PixmapFormat::PixelFormat id)
      : Message(type_dpy), fmt_id(id) {};

    virtual ~Dpy(){}

  };


  class Resize : public Message {

  public:
    int    width;
    int    height;

    Resize() 
      : Message(type_resize),width(0),height(0) {}

    Resize(int width, int height)
      : Message(type_resize), width(width),height(height) {}

    virtual ~Resize(){}
  };

  class PictureFrame : public Message {

  public:
    static void resetSeq(){seqNoCounter = 0;};
    static int             seqNoCounter;
    int                           seqNo;
    VideoFrame*                 picture;

    PictureFrame() 
      : Message(type_frame), seqNo(0), picture(0) {}
    
    PictureFrame(VideoFrame* frame)
      : Message(type_frame), seqNo(0), picture(frame) {seqNo = seqNoCounter++;}

    virtual ~PictureFrame() {}

  };

  class SetDpySize : Message {
    
  public:
    unsigned int                dpy_fmtid;
    int                             width;
    int                            height;

    SetDpySize() 
      : Message(type_dpySize),dpy_fmtid(0),width(0),height(0) {}

    SetDpySize(unsigned int fmtid, int width, int height)
      : Message(type_dpySize), dpy_fmtid(fmtid),width(width),height(height) {};

    virtual ~SetDpySize() {}

  };

}
#endif

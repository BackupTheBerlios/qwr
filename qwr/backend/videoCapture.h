/*
 * abstract Video Capture Class 
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

#ifndef VIDEO_CAPTURE_H
#define VIDEO_CAPTURE_H

#include <string>

#include"pixmapFormat.h"
#include"videoFrame.h"

class VideoCapture 
{
    struct FrameBuffer
    {

    };

 private:
   

 protected:
    char*                          devname;
    int                            fd;

    /* capture */
    int                            fps;
    unsigned int                   frame_count;
    int                            first;
    unsigned int                   start;
    int                            method; /* use_read */

    PixmapFormat                   curr_fmt;

 public:
    VideoCapture();
    virtual ~VideoCapture();
    virtual void   init();
    unsigned int   timestamp();

    /* open/close */
    virtual int    open(char *device) = 0;
    virtual int    close() = 0;

    /* attributes */
    virtual char*  get_devname();
    virtual int    can_capture();
    virtual void   get_device_capabilities();
    
    
    /* capture */
    virtual int          setformat(PixmapFormat& in_fmt) = 0;
    virtual void         setPictureParams(int colour, int brightness,
					  int hue,    int contrast) = 0;
    virtual int          startvideo(int fps, unsigned int buffers) = 0;
    virtual void         stopvideo() = 0;
    virtual VideoFrame*  nextframe() = 0;
};


#endif  // VIDEO_CAPTURE_H

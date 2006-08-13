/*
 * video for linux interface
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
#include"VideoFrame.h"

#define READ_IO_METHOD 0
#define MMAP_IO_METHOD 1
#define UPTR_IO_METHOD 2

class VideoCapture 
{
 private:

 protected:
    char const*                    devname;
    int                            fd;

    int                            fps;
    PixmapFormat                   curr_fmt;

    int                            method;

    unsigned int                   frame_count;
    unsigned int                   start; /* timestamp start */

 public:
    VideoCapture(char const* device);
    virtual ~VideoCapture();

    virtual int          open() = 0;
    virtual int          close() = 0;

    virtual char const*  get_devname() = 0;
    virtual void         get_device_capabilities() = 0;
    
    virtual int          setFormat(PixmapFormat& in_fmt) = 0;
    virtual void         setPictureParams(int colour, int brightness,
					  int hue,    int contrast) = 0;
    virtual int          startVideo(int fps, unsigned int buffers) = 0;
    virtual void         stopVideo() = 0;
    virtual VideoFrame*  nextFrame() = 0;
};

#endif  // VIDEO_CAPTURE_H

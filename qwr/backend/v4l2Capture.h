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

#ifndef V4L2_CAPTURE_H
#define V4L2_CAPTURE_H

#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#include <asm/types.h>          /* for videodev2.h */
#include <linux/compiler.h>     /* for videodev2.h */
#include <linux/videodev2.h>

#include"videoCapture.h"


class v4l2Capture : public VideoCapture
{

 private:
    static const unsigned char     wanted_buffers = 32;
    static const unsigned char     max_format     = 32;

    /* device descriptions */
    int                            nfmts;
    struct v4l2_capability	   cap;
    struct v4l2_streamparm	   streamparm;
    struct v4l2_fmtdesc		   fmt[max_format];


    /* capture */

    struct v4l2_format             fmt_v4l2;
    struct v4l2_requestbuffers     reqbufs;
    struct v4l2_buffer             buf_v4l2[wanted_buffers];
    struct VideoFrame              buf_me[wanted_buffers];
    unsigned int                   queue;
    unsigned int                   waiton;

 protected:
    int    xioctl(int fd, int cmd, void *arg, int mayfail);
    int    queue_buffer(void);
    void   queue_all(void);

 public:
    v4l2Capture(char const* device = "/dev/video0");
    ~v4l2Capture();

    /* open/close */
    virtual int    open();
    virtual int    close();

    /* attributes */
    virtual char const*  get_devname();
    virtual void         get_device_capabilities();
    
    /* capture */
    virtual int          setFormat(PixmapFormat& in_fmt);
    virtual void         setPictureParams(int colour, int brightness,
					  int hue,    int contrast);
    virtual int          startVideo(int fps, unsigned int buffers);
    virtual void         stopVideo();
    virtual VideoFrame*  nextFrame();
};


#endif  // V4L2_CAPTURE_H

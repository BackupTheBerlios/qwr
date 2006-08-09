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

#ifndef V4L_CAPTURE_H
#define V4L_CAPTURE_H

#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>


#include <linux/videodev.h>

#include"videoCapture.h"


class v4lCapture : public VideoCapture
{

 private:

    /* device descriptions */
    struct video_capability        capability;
    struct video_channel           *channels;
    struct video_tuner             tuner;
    struct video_audio             audio;
    struct video_picture           pict;



    /* capture to mmap()'ed buffers */
    struct video_mbuf              mbuf;
    unsigned char                  *mmap;
    unsigned int                   nbuf;
    unsigned int                   queue;
    unsigned int                   waiton;

    int                            probe[19]; //VIDEO_FMT_COUNT
    struct video_mmap              *buf_v4l;
    struct VideoFrame              *buf_me;


    /* capture via read() */
    PixmapFormat                   rd_fmt;
    struct video_window            rd_win;

 protected:
    static std::map<PixmapFormat::PixelFormat, unsigned short> v4l_pix_id;
    static std::map<unsigned short, PixmapFormat::PixelFormat> ptt_pix_id;
    void   init_palette_table(void);

    int    xioctl(int fd, int cmd, void *arg);
    bool   is_format_supported(unsigned int fmtid);
    int    queue_buffer(void);
    void   queue_all(void);

    int    mm_waiton(void);
    void   mm_clear(void);

    int    mm_setparams(PixmapFormat& fmt);

 public:
    v4lCapture();
    ~v4lCapture();
    virtual void   init();

    /* open/close */
    virtual int    open(char *device);
    virtual int    close();

    /* attributes */
    virtual char*  get_devname();
    virtual int    can_capture();
    virtual void   get_device_capabilities();
    
    
    /* capture */
    virtual int          setformat(PixmapFormat& in_fmt);
    virtual void         setPictureParams(int colour, int brightness,
					  int hue,    int contrast);
    virtual int          startvideo(int fps, unsigned int buffers);
    virtual void         stopvideo();
    virtual VideoFrame*  nextframe(); /* video frame */
};


#endif  // V4L_CAPTURE_H

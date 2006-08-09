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

#include"videoCapture.h"


VideoCapture::VideoCapture()
    : fd(0),
      method(1),
      curr_fmt(PixmapFormat::PIXFMT_YUV420P, 0, 0)
{
    fprintf(stderr, "VideoCapture::VideoCapture()\n");
    devname = "/dev/video0";
}


VideoCapture::~VideoCapture()
{
    fprintf(stderr, "VideoCapture::~VideoCapture()\n");

   
}

void VideoCapture::init()
{
    fprintf(stderr, "VideoCapture::init()\n");
    get_device_capabilities();
}

void VideoCapture::get_device_capabilities()
{

}

char* VideoCapture::get_devname()
{
    fprintf(stderr, "VideoCapture::get_devname()\n");
    return devname; 
}

int VideoCapture::can_capture()
{
    fprintf(stderr, "VideoCapture::can_capture()\n");
    return 0;
}




#include <sys/time.h>
unsigned int VideoCapture::timestamp()
{
    struct timeval tv;
    gettimeofday(&tv,NULL);
    return ((tv.tv_sec * 1000000 + tv.tv_usec) / 1000);
}












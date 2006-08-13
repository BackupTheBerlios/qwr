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

#include <iostream>

#include"videoCapture.h"

VideoCapture::VideoCapture(char const* device)
    : devname(device),
      fd(0),
      curr_fmt(PixmapFormat::PIXFMT_YUV420P, 0, 0),
      method(READ_IO_METHOD)
{
    std::cerr<<"VideoCapture::VideoCapture()\n";
}


VideoCapture::~VideoCapture()
{
    std::cerr<<"VideoCapture::~VideoCapture()\n";  
}

















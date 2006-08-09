/*
 * conversion between different pixmap formats
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

#ifndef convert_h
#define convert_h

#include <map>

#include "avcodec.h"

#include"pixmapFormat.h"
#include"videoFrame.h"

//! this schould be an abstract class
class Convert {

 protected:
  std::map<PixmapFormat::PixelFormat, enum PixelFormat> av_pix_id;
  std::map<enum PixelFormat, PixmapFormat::PixelFormat> ptt_pix_id;

  /* conversion definitions */
  AVFrame*             in_conv_frame;
  AVFrame*             out_conv_frame;
 
 public:
  Convert();
  virtual ~Convert();

  VideoFrame* convert(VideoFrame* inFrame, PixmapFormat::PixelFormat from);

};

#endif

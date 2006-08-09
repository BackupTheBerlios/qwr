/*
 * pixmap info class 
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

#include"pixmapFormat.h"

#include<stdio.h>


std::map<PixmapFormat::PixelFormat,unsigned int> PixmapFormat::depth_table;
bool PixmapFormat::depth_table_initialized = false;


PixmapFormat::PixmapFormat()
    : _id(PIXFMT_NONE), _width(0), _height(0), _bytesperline(0)
{
  //    fprintf(stderr, "Pixmapformat::Pixmapformat()\n");
    if(!depth_table_initialized)
	init_depth_table();
}

PixmapFormat::~PixmapFormat()
{
}

PixmapFormat::PixmapFormat(PixelFormat id, unsigned int w, unsigned int h)
    : _id(id), _width(w), _height(h)
{
  //fprintf(stderr, "Pixmapformat::Pixmapformat()\n");
//  _bytesperline  = _width * depth_table[_id] / 8;
    _bytesperline  = _width * this->depth() / 8;
}

PixmapFormat::PixmapFormat(const PixmapFormat& fmt)
{
    _id            = fmt._id;
    _width         = fmt._width;
    _height        = fmt._height;
    _bytesperline  = fmt._bytesperline;
}


PixmapFormat& PixmapFormat::setFormat(const PixmapFormat& fmt)
{
    _id            = fmt.id();
    _width         = fmt.width();
    _height        = fmt.height();
    _bytesperline  = fmt.linesize();

    return *this;
}


PixmapFormat&
PixmapFormat::setFormat(const PixelFormat id,
			unsigned int w,
			unsigned int h)
{
  //    fprintf(stderr, "Pixmapformat::setFormat()\n");
    _id            = id;
    _width         = w;
    _height        = h;
//  _bytesperline  = _width * depth_table[_id] / 8;
    _bytesperline  = _width * this->depth() / 8;

    return *this;
}


PixmapFormat& PixmapFormat::operator= (const PixmapFormat& fmt)
{
    if (this != &fmt) {
	_id            = fmt._id;
	_width         = fmt._width;
	_height        = fmt._height;
	_bytesperline  = fmt._bytesperline;
    }

    return *this;
}

bool PixmapFormat::operator== (const PixmapFormat& fmt) const
{

    return (_id             == fmt._id &&
	    _width          == fmt._width &&
	    _height         == fmt._height &&
	    _bytesperline   == fmt._bytesperline);
}

bool PixmapFormat::operator!= (const PixmapFormat& fmt) const
{
    return !(*this == fmt);
}

unsigned int PixmapFormat::depth()
{
    switch(_id)
    {
	case PIXFMT_RGB15_LE:
	case PIXFMT_RGB16_LE:
	case PIXFMT_RGB15_BE:
	case PIXFMT_RGB16_BE:
	    return 16;
	    
	case PIXFMT_BGR24: 
	case PIXFMT_RGB24:
	    return 24;

	case PIXFMT_BGR32:
	case PIXFMT_RGB32:
	    return 32;

	case PIXFMT_YUYV:
	case PIXFMT_YUV422P:
	case PIXFMT_UYVY:
	    return 16;

	case PIXFMT_YUV420P:
	    return 12;

	default:
	    return 0;
    }
    return 0;
}

void PixmapFormat::print(void)
{
    fprintf(stderr,"Pixmapformat: \n"
	    "id        = %d \n"
	    "width     = %d \n"
	    "height    = %d \n"
	    "linesize  = %d \n",
	    _id, _width, _height, _bytesperline);
}


//typedef std::map<PixmapFormat::PixelFormat,unsigned int>::value_type value_type;
void PixmapFormat::init_depth_table(void)
{
    typedef std::map<PixelFormat,unsigned int>::value_type value_type;

    depth_table.insert(value_type(PixelFormat(PIXFMT_NONE), 0));
    
    depth_table.insert(value_type(PixelFormat(PIXFMT_RGB15_LE), 16));
    depth_table.insert(value_type(PixelFormat(PIXFMT_RGB16_LE), 16));
    depth_table.insert(value_type(PixelFormat(PIXFMT_RGB15_BE), 16));
    depth_table.insert(value_type(PixelFormat(PIXFMT_RGB16_BE), 16));
    

    depth_table.insert(value_type(PixelFormat(PIXFMT_BGR24), 24));
    depth_table.insert(value_type(PixelFormat(PIXFMT_BGR32), 32));
    depth_table.insert(value_type(PixelFormat(PIXFMT_RGB24), 24));
    depth_table.insert(value_type(PixelFormat(PIXFMT_RGB32), 32));

    depth_table.insert(value_type(PixelFormat(PIXFMT_YUYV),    16));
    depth_table.insert(value_type(PixelFormat(PIXFMT_YUV422P), 16));
    depth_table.insert(value_type(PixelFormat(PIXFMT_YUV420P),12));
    depth_table.insert(value_type(PixelFormat(PIXFMT_UYVY),    16));

    depth_table_initialized = true;

/*
    depth_table[PIXFMT_NONE]     = 0;

    depth_table[PIXFMT_RGB15_LE] = 16;
    depth_table[PIXFMT_RGB16_LE] = 16;
    depth_table[PIXFMT_RGB15_BE] = 16;
    depth_table[PIXFMT_RGB16_BE] = 16;

    depth_table[PIXFMT_BGR24]    = 24;
    depth_table[PIXFMT_BGR32]    = 32;
    depth_table[PIXFMT_RGB24]    = 24;
    depth_table[PIXFMT_RGB32]    = 32;

    depth_table[PIXFMT_YUYV]     = 16;
    depth_table[PIXFMT_YUV422P]  = 16;
    depth_table[PIXFMT_YUV420P]  = 12;

    depth_table[PIXFMT_UYVY]     = 16;
*/
}

/*
PixmapFormat& PixmapFormat::setFormat(PixelFormat id, unsigned int w,
				      unsigned int h, unsigned int ls)
{
    fprintf(stderr, "Pixmapformat::setFormat()\n");
    _id            = id;
    _width         = w;
    _height        = h;

    fprintf(stderr, "depth_table.size() = % d\n", depth_table.size());
    std::map<enum PixelFormat,unsigned int>::iterator iter = depth_table.begin();
    if( (iter = depth_table.find(std::map<enum PixelFormat,unsigned int>::key_type(_id))) ==
	depth_table.end() )
    {
	fprintf(stderr, "id to depth not found \n");
    }
    else
    {
	int depth = iter->second;
	fprintf(stderr, "depth = % d\n", depth);
    }
    

    _bytesperline  = _width * depth_table[_id] / 8;

    fprintf(stderr, "id = % d\n", _id);
    fprintf(stderr, "depth = % d\n", depth_table[_id]);
    fprintf(stderr, "_width = % d\n", _width);
    fprintf(stderr, "_bytesperline = % d\n", _bytesperline);

//  _bytesperline  = ls;

    return *this;
}
*/

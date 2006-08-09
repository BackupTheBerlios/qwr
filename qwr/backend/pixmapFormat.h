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

#ifndef PIXMAP_FORMAT_H
#define PIXMAP_FORMAT_H

#include <string>
#include <vector>
#include <map>

class PixmapFormat 
{
 public:
    typedef enum {
	PIXFMT_NONE     =       0,
        PIXFMT_RGB15_LE =       1,
	PIXFMT_RGB16_LE =       2,
        PIXFMT_RGB15_BE =       3,
        PIXFMT_RGB16_BE =       4,
	PIXFMT_BGR24    =       5,
	PIXFMT_BGR32    =       6,
        PIXFMT_RGB24    =       7,
        PIXFMT_RGB32    =       8,
        PIXFMT_YUYV     =      19,
        PIXFMT_YUV422P  =      10,
	PIXFMT_YUV420P  =      11,
        PIXFMT_UYVY     =      12
    } PixelFormat;

 private:
    PixelFormat    _id;
    unsigned int   _width;
    unsigned int   _height;
    unsigned int   _bytesperline;

 protected:
    static std::map<PixelFormat,unsigned int> depth_table;
    static bool depth_table_initialized;
    void        init_depth_table(void);


 public:
    PixmapFormat();
    ~PixmapFormat();
    PixmapFormat(PixelFormat id, unsigned int w, unsigned int h);
    PixmapFormat(const PixmapFormat& fmt);

    PixelFormat  id()       const  {return _id;};
    unsigned int width()    const  {return _width;};
    unsigned int height()   const  {return _height;};
    unsigned int linesize() const  {return _bytesperline;};
    unsigned int size()     const  {return _bytesperline * _height;};
    unsigned int depth();
    void         print();

    PixmapFormat& setFormat(const PixmapFormat& fmt);
    PixmapFormat& setFormat(const PixelFormat id,
			    unsigned int w = 0,
			    unsigned int h = 0);

    PixmapFormat& operator= (const PixmapFormat& fmt);
    
    bool operator==(const PixmapFormat& fmt) const;
    bool operator!=(const PixmapFormat& fmt) const;
};


#endif  // PIXMAP_FORMAT_H

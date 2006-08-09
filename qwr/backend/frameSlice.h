/*
 * repository for one picture
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

#ifndef frameSlice_h
#define frameSlice_h

class FrameSlice {

 private:
  bool         copy;     //!< is this a copy?
  const char*  data;     //!< frame slice data
  unsigned int length;   //!< frame slice data length

 public:
  FrameSlice(const char* data, const unsigned int length, bool copy = true);
  virtual ~FrameSlice(); 

  const char*  data();
  unsigned int length();
};

inline FrameSlice::FrameSlice(const char* _data, 
			      const unsigend int _length, 
			      bool _copy = true)
{
  copy   = _copy;
  length = _length;
  data = (copy ? (char*)memcpy(new char[length], _data, length) : _data);
}

inline FrameSlice::~FrameSlice()
{ 
  if(copy) 
    delete data; 
} 

inline const char* FrameSlice::data()
{
  return (data);
}

inline unsigned int FrameSlice::length()
{
  return (length);
}


#endif

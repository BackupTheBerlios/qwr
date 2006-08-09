/*
 * simple ringbuffer class
 * Copyright (C) 2005-2006 Joern Seger 
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

#include "ringbuffer.h"

#include <iostream>

ringbuffer::ringbuffer(unsigned int buffersize)
  : size(buffersize), used(0), begin(0), end(0)
{
  fifo = new char[buffersize];  
}

ringbuffer::~ringbuffer()
{
  delete fifo;
}

unsigned int ringbuffer::addData(char* data, unsigned int len)
{ 
  /*
  if (used+len > size)
    return 0; // man koennte auch so viel wie moeglich queuen ...
  */
  bool drop(false);
  // not fast, but ...
  for (unsigned int i= 0; i<len; ++i) {

    // this is used, if overwriting is used
    if ((used) && (begin == end)) {
      end++;
      end %= size;
      drop = true;
    }
    else
      used++;

    fifo[begin] = data[i];
    begin++;
    begin %= size;
  }

  if (drop)
    std::cerr<<"d";

  return (len);

}

unsigned int ringbuffer::getData(char*data, unsigned int len)
{
  if (used < len)
    len = used;

  for(unsigned int i=0; i<len; ++i) {
    data[i] = fifo[end++];
    end %= size;
  }
  used -= len;

  return (len);
}

unsigned int ringbuffer::getAvailable()
{
  return(size-used);
}

unsigned int ringbuffer::getUsed()
{
  return(used);
}

void ringbuffer::clean()
{
  begin = end = used = 0;
}

unsigned int ringbuffer::luenkerback(char* data, unsigned int len)
{
  if (len>used)
    len = used;

  int tmpEnd = begin-1;
  for (int i=len-1; i>=0; --i){
    if (tmpEnd < 0)
      tmpEnd = size-1;
    data[i] = fifo[tmpEnd--];    
  }
  return (len);
}

unsigned int ringbuffer::luenkerfront(char* data, unsigned int len)
{
  if (used < len)
    len = used;

  unsigned int tmpEnd = end;
  for(unsigned int i=0; i<len; ++i) {
    data[i] = fifo[tmpEnd++];
    tmpEnd %= size;
  }
  //  used -= len;

  return (len);
}

unsigned int ringbuffer::inc(unsigned int len)
{
  if (used < len)
    len = used;

  end += len;
  end %= size;
  used -= len;

  return (len);  
}

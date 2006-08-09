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

#ifndef ringbuffer_h
#define ringbuffer_h

class ringbuffer {

 protected:
  char* fifo;

  unsigned int size;
  unsigned int used;
  unsigned int begin; //! first available sign
  unsigned int end;   //! oldest packet

 public:
  ringbuffer(unsigned int buffersize = 8000);
  virtual ~ringbuffer();

  unsigned int addData(char* data, unsigned int len);
  unsigned int getData(char* data, unsigned int len);

  unsigned int getAvailable();
  unsigned int getUsed();

  // read newest nBytes
  unsigned int luenkerback(char* data, unsigned int len); 
  
  // read oldest nBytes
  unsigned int luenkerfront(char* data, unsigned int len); 
  
  // delete the oldes len bytes
  unsigned int inc(unsigned int len);

  void clean();
};

#endif

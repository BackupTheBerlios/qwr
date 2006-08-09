/*
 * header information for ogg
 * Copyright (C) 2006 Joern Seger 
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

#ifndef oggHeader_h
#define oggHeader_h

#include "../esystem/definitions.h"
struct OggHeader {

  char    ogg[5];

  char    pack_type:1;
  char    page_type:1;
  char    last:1;
  char    reserved:5;

  int64   position;
  uint32  serial;
  uint32  pageNo;
  uint32  checksum;
  uint8   tableSegments;

} __attribute__ ((packed));

struct StreamType {
  unsigned char    headerType;
  char    typeName[6];
}__attribute__ ((packed));

// don't forget to convert the values
// to the right order 

struct TheoraHeader {
  char    vmaj;
  char    vmin;
  char    vrev;
  uint16  fmbw;
  uint16  fmbh;
  //  uint32  nsbs;
  //  uint64  nbs1:36;
  //uint32  nmbs;
  uint32  picw:24;
  uint32  pich:24;
  char    picx;
  char    picy;
  uint32  frn;
  uint32  frd;
  uint32  parn:24;
  uint32  pard:24;
  char    cs;
  uint32  nombr:24;

  // its little endian
  // and network byte order !!!
  union {
    struct {
      uint16  reserved:3;
      uint16  pf:2;
      uint16  kfgshift:5;
      uint16  qual:6;
    } blub;
    uint16 pleaseconvert;
  } f;

  /*
  uint16  qual:6;
  uint16  kfgshift:5;
  uint16  pf:2;
  uint16  reserved:3;
  */
} __attribute__ ((packed));

struct VorbisHeader {
  uint32 version;
  uint8  audioChannels;
  uint32 sampleRate;
  uint32 bitrateMax;
  uint32 bitrateNom;
  uint32 bitrateMin;
  uint8  blocksize0:4;
  uint8  blocksize1:4;
  uint8  framing; //?
}__attribute__ ((packed));


// alignment is really wired
struct VorbisMode {
  uint8  blockFlag:1;
  uint16 windowType; // MUST be zero
  uint16 transformType; // MUST be zero
  uint8  mapping;
  uint8  framing:1;
}__attribute__ ((packed));

//

#endif

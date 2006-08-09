/*
 * some definitions for the esystem
 * Copyright (c) 2005-2006 Joern Seger
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef definitions_h
#define definitions_h

#define HZ 1000

#ifndef WITH_CCPP
typedef unsigned long long uint64;
typedef unsigned int       uint32;
typedef unsigned short     uint16;
typedef unsigned char      uint8;
typedef long long          int64;
typedef int                int32;
typedef short              int16;
typedef char               int8;
#else
#include <cc++/config.h>
#endif

#endif

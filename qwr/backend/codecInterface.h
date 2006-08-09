/*
 * abstract codec interface
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

#ifndef codecInterface_h
#define codecInterface_h

#include "connection.h"
#include "message.h"
#ifdef WITH_QT
#include "qTask.h"
#else
#include "itask.h"
#endif

#include "codecInfo.h"
#include "codecMessage.h"

//! Main interface for all Codecs
/*! In difference to other codec code, this is 
  fixed to only _one_ data stream which consists of _one_ audio
  stream and _one_ video stream! 
  The raw video data in yuv 4:2:0 is transfered to the codec through 
  addRawVideoSample(). The raw pcm audio samples are transfered 
  addRawAudioSample(). 
  The video samples MUST fit to the specifications given by the 
  initCodec() method (see CodecInfo for more information).

  This will not be scheduled
 */

#ifdef WITH_QT
class CodecInterface : public QTask {
#else
class CodecInterface : public ITask {
#endif
 protected:
  CodecInfo* codecInfo;
  Connection* connection;

  bool encoding;
  bool decoding;

  virtual void eventHandler(Event event);

  virtual void addRawVideoSample(char* videoData, unsigned int length) = 0; 
  virtual void addRawAudioSample(float* audioData, unsigned int length) = 0;

  virtual void addDecoderPacket(char* data, unsigned int length) = 0;

  //! initializing the Codec
  /*! @return true on success; false if the init failed */
 
  virtual bool startDecoder() = 0;
  virtual bool stopDecoder() = 0;

  virtual void transferData(unsigned char* data, unsigned int length, bool header = false);

 public:
  CodecInterface();
  virtual ~CodecInterface();

  virtual bool startEncoder(CodecInfo* info) = 0;
  virtual bool stopEncoder() = 0;

  virtual bool connectDecoderPipe(Connection& connection);

};

#endif

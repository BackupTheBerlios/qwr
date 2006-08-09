/*
 * messages between codec and application
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

#ifndef codecMessage_h
#define codecMessage_h

#include "codecInfo.h"

class CodecMessage : public Message {
  
 public:
  enum MessageID {
    none,
    startEncoder,
    stopEncoder,
    startDecoder,
    stopDecoder,
    rawVideoData,
    rawAudioData,
    decodedData,
    decodedHeader
  };

  CodecMessage(MessageID ID = none);
  virtual ~CodecMessage();

};

class CodecDataMessage : public CodecMessage {

 public:
  char*          data;
  unsigned int   length;

  CodecDataMessage(MessageID ID, char* data, unsigned int length);
  virtual ~CodecDataMessage();

};

class StartEncoderMessage : public CodecMessage {

 public:
  CodecInfo* info;
  
  StartEncoderMessage(CodecInfo* info);
  virtual ~StartEncoderMessage();

};

class StopEncoderMessage: public CodecMessage {

 public:
  StopEncoderMessage();
  virtual ~StopEncoderMessage();

};

class StartDecoderMessage: public CodecMessage {

 public:
  StartDecoderMessage();
  virtual ~StartDecoderMessage();

};

class StopDecoderMessage : public CodecMessage {
  
 public:
  StopDecoderMessage();
  virtual ~StopDecoderMessage();

};

/**************** INLINE SPECIFICATIONS *********************************/

inline CodecMessage::CodecMessage(MessageID ID)
  : Message(ID)
{
}

inline CodecMessage::~CodecMessage()
{
}

//-------------------------------------------------

inline CodecDataMessage::CodecDataMessage(MessageID ID, char* _data, unsigned int _length)
  : CodecMessage(ID)
{
  if (!_length) {
    data   = 0;
    length = 0;
    return;
  }
 
  length = _length;
  memcpy(data = new char[length],_data, length);
}

inline CodecDataMessage::~CodecDataMessage()
{
  delete data;
}

//-------------------------------------------------

inline StartEncoderMessage::StartEncoderMessage(CodecInfo* _info)
  : CodecMessage(CodecMessage::startEncoder), info(_info)
{
}

inline StartEncoderMessage::~StartEncoderMessage()
{}

//-------------------------------------------------

inline StopEncoderMessage::StopEncoderMessage()
  : CodecMessage(CodecMessage::stopEncoder)
{}

inline StopEncoderMessage::~StopEncoderMessage()
{}

//-------------------------------------------------

inline StartDecoderMessage::StartDecoderMessage()
  : CodecMessage(CodecMessage::startDecoder)
{}

inline StartDecoderMessage::~StartDecoderMessage()
{}

//-------------------------------------------------

inline StopDecoderMessage::StopDecoderMessage()
  : CodecMessage(CodecMessage::stopDecoder)
{}

inline StopDecoderMessage::~StopDecoderMessage()
{}

#endif

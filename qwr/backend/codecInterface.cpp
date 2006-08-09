/*
 * abstract Codec Interface
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

#include "codecInterface.h"

//#include <iostream>

CodecInterface::CodecInterface()
  : codecInfo(0), connection(0), encoding(false), decoding(false)
{
}

CodecInterface::~CodecInterface()
{
}

bool CodecInterface::connectDecoderPipe(Connection& _connection)
{
  connection = &_connection;
  connection->addTask(this, 0, scheduler); // there will only be one pipe, where we receive from

  return(true);
}

void CodecInterface::eventHandler(Event event)
{
  Message* msg;
  connection->receiveMessage(msg, this);
  //  std::cerr << " CI("<<msg->getID()<<") ";
  switch(msg->getID()) {

  case CodecMessage::rawVideoData: {
    if (!encoding)
      break;
    CodecDataMessage* message = static_cast<CodecDataMessage*>(msg);
    addRawVideoSample(message->data, message->length);
  } break;

  case CodecMessage::rawAudioData: {
    if (!encoding)
      break;
    CodecDataMessage* message = static_cast<CodecDataMessage*>(msg);
    addRawAudioSample((float*)message->data, message->length/(sizeof(float)));
  } break;

  case CodecMessage::startEncoder: {
    if (encoding || decoding) 
      break;
    encoding = true;
    StartEncoderMessage* message = static_cast<StartEncoderMessage*>(msg);    
    startEncoder(message->info);
  } break;

  case CodecMessage::stopEncoder: {
    if (!encoding)
      break;
    encoding = false;
    stopEncoder();
  } break;

  case CodecMessage::startDecoder: {
    if (encoding || decoding)
      break;
    decoding = true;
    startDecoder();
  } break;

  case CodecMessage::stopDecoder: {
    if (!decoding)
      break;
    decoding = false;
    stopDecoder();
  } break;

  case CodecMessage::decodedData: {
    if (!decoding)
      break;
    CodecDataMessage* message = static_cast<CodecDataMessage*>(msg);
    addDecoderPacket(message->data, message->length);
  } break;

  }

  delete msg;
}

void CodecInterface::transferData(unsigned char* data, unsigned int length, bool header)
{
  CodecMessage::MessageID ID = (header?CodecMessage::decodedHeader:CodecMessage::decodedData);
  
  CodecDataMessage* msg = new CodecDataMessage(ID, (char*)data, length);
  connection->sendMessage(msg, this);
}


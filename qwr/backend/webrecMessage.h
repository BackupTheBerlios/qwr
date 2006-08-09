/*
 * messages between backend and GUI
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

#ifndef webrecMessage_h
#define webrecMessage_h

/*
  Brainstorming, what do we need:

  placed in one message:
  - start record
  - stop record
  - start play
  - stop play
  - start preview
  - stop preview

  special messages:
  - set filename
  - get filename

  - set codec information
  - get codec information

  - resize

*/

#include "message.h"

class WebrecMessage : public Message {

 public:

  enum MessageType {
    play,
    record,
    stream,
    preview,
    filename,
    streamname,
    codecInfo
  };

  enum TransferType {
    BackendSet,
    BackendUnset,
    BackendGet,
    GuiSet,
    GuiUnset,
    GuiGet
  };

 protected:

 public:
  //  MessageType  msgType;
  TransferType trType;

  WebrecMessage(MessageType _msgType, TransferType _trType)
    : Message(_msgType), trType(_trType) {}

  virtual ~WebrecMessage() {}

};

class CodecInfoMessage : public WebrecMessage {

 public:
  enum MediaType {
    audio,
    video
  };

  MediaType mediaType;
  int       codecType;
  int       codecBitrate;

  CodecInfoMessage(TransferType _trType, MediaType _mediaType, int _codecType, int _codecBitrate)
    : WebrecMessage(codecInfo, _trType), mediaType(_mediaType), codecType(_codecType),
      codecBitrate(_codecBitrate) {};

  virtual ~CodecInfoMessage(){};

};

class FileInfoMessage : public WebrecMessage {

 public:
  std::string filename;

  FileInfoMessage(TransferType _trType, std::string _filename)
    : WebrecMessage(WebrecMessage::filename, _trType), filename(_filename) {};

  virtual ~FileInfoMessage(){};

};

class StreamInfoMessage : public WebrecMessage {

 public:
  std::string streamname;

  StreamInfoMessage(TransferType _trType, std::string _streamname)
    : WebrecMessage(WebrecMessage::streamname, _trType), streamname(_streamname) {};

    virtual ~StreamInfoMessage(){};

};

//class SetGet

#endif

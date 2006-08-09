/*
 * ogg/theora/vorbis converter class 
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

#ifndef theoraCodec_h
#define theoraCodec_h

#include <theora/theora.h>
#include <vorbis/codec.h>
#include <vorbis/vorbisenc.h>

#include "timev.h"
#include "codecInterface.h"

/* The handling is as follows:

1) start the decoder/encoder with the message
startEncoderMessage/startDecoderMessage

2) send message via RawVideoMessage/RawAudioMessage/DecodeMessage

3) close the Encoder by stopEncoder/stopDecoder

now it is getting complicated: there might be unhandeled packets
waiting in the queue, so we gonna send another event, which will be
placed directly behind all other event. This event indicates, that
there will be no more packets arriving. 
SO: when the stopEncoder is called, there MUST NOT be any more
packtes been send to the decoder/encoder
*/

class TheoraCodec : public CodecInterface {

 protected:
  enum EventID_t {
    audioEvent,
    videoEvent
  };

  // tbd: thinking about taking this to codecInterface
  static const unsigned int fini = 1; 

  // theora needs special width and heigth
  int videoWidth;
  int videoHeight;

  int frameWidth;   // smaller than videoWidth
  int frameHeight;  // smaller than videoHeight

  int videoFrameOffsetWidth;
  int videoFrameOffsetHeight;

  theora_info      theoraInfo;
  vorbis_info      vorbisInfo;

  // ogg data
  ogg_stream_state theoraOggStream;
  ogg_stream_state vorbisOggStream;
  ogg_sync_state   oggSyncState;
  ogg_page         oggBitstreamPage;
  ogg_page         oggPage;
  ogg_packet       oggPacket; 

  // Theora specific data
  theora_state     theoraState;
  theora_comment   theoraComment;

  // vorbis data
  vorbis_comment   vorbisComment;

  vorbis_dsp_state vorbisState;
  vorbis_block     vorbisBlock;

  // ogg pages for audio and video
  ogg_page         audioPage;
  ogg_page         videoPage;

  // two video frames
  signed char* yuvframe[2];

  int togg; // toggle bit

  // decoder only:
  bool headerEncoded;

  timev startAudioTime;
  timev startVideoTime;

  void handleFini();
  void parseHeader();

  virtual void addRawVideoSample(char* videoData, unsigned int length);
  virtual void addRawAudioSample(float* audioData, unsigned int length);

  virtual void addDecoderPacket(char* data, unsigned int length);

  virtual bool startDecoder(); //{return(true);};
  virtual bool stopDecoder(); //{return(true);};

  virtual void eventHandler(Event event);

  void audioDecode();
  void videoDecode();

 public:
  TheoraCodec();
  virtual ~TheoraCodec();

  virtual bool startEncoder(CodecInfo* info);
  virtual bool stopEncoder();

};

#endif

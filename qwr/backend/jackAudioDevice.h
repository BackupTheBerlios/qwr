/*
 * wrapper class for the jack audio device
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

#ifndef jackAudioDevice_h
#define jackAudioDevice_h

#include <jack/jack.h>
#include <jack/ringbuffer.h>

#include "audioDevice.h"

extern "C" int processRec(jack_nframes_t nframes, void *arg);

class JackAudioDevice : public AudioDevice {

 public:
  enum ClientState_t {
    Init,
    Idle,
    RunPlay,
    RunRec,
    RunDuplex,
    Exit
  };

 protected:

  static const uint32 defaultRingBufferSize = 48000; // 250ms

  volatile ClientState_t clientState;

  jack_client_t* jackClientHandle; 
  
  // identifier to connect/disconnect
  char* phyCapturePort;
  char* phyOutputPort;
  char* appCapturePort;
  char* appOutputPort;

  Event ownRecEvent;

  static const uint32 recCallInterval = (int)(0.02*HZ);

 public:

  // must be public because of callbacks
  jack_ringbuffer_t* recRingBuffer;
  jack_ringbuffer_t* playRingBuffer;

  // identifier to read/write
  jack_port_t* inputPort;
  jack_port_t* outputPort;

  bool isState(ClientState_t stat);
  ClientState_t getState();
  bool changeState(ClientState_t stat);

  jack_client_t* getClientHandle();

  JackAudioDevice(Scheduler* _scheduler, Event _recEvent);
  virtual ~JackAudioDevice();

  virtual bool openAudioPlayDevice();
  virtual void closeAudioPlayDevice();

  virtual bool openAudioRecDevice();
  virtual void closeAudioRecDevice();

  virtual bool fillPlayBuffer(float* data, uint16 length);
  virtual bool receiveRecording(float* data, uint16 &length);

  virtual uint16 receiveLength(); 

  virtual void eventHandler(Event event);

};

#endif

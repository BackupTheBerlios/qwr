/*
 * abstract audio device class
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

#ifndef audioDevice_h
#define audioDevice_h

#include "task.h"
//#include "audioDeviceMessage.h"

// abstract interface class, to handle audio devices
class AudioDevice : public Task {

 protected:
  Event recEvent;

  volatile unsigned int sampleRate;

 public:
  enum audioDeviceEvent {
    open,         // open device -> actually not used 
    close,        // close device -> actually not used
    timeout_rec,  // record timeout 
    rcvPlayData   // data have been received to play
  };

  static const uint16 maxAudioDataLength = 10000;

  AudioDevice(Scheduler* _scheduler, Event _recEvent);
  virtual ~AudioDevice();

  virtual bool openAudioPlayDevice() = 0;
  virtual void closeAudioPlayDevice() = 0;

  virtual bool openAudioRecDevice() = 0;
  virtual void closeAudioRecDevice() = 0;

  unsigned int getSampleRate();

  virtual bool fillPlayBuffer(float* data, uint16 length) = 0;
  virtual bool receiveRecording(float* data, uint16 &length) = 0;
  virtual uint16 receiveLength() = 0;

  //  virtual void eventHandler(Event event);

};

inline AudioDevice::AudioDevice(Scheduler* _scheduler, Event _recEvent)
  : Task(_scheduler), recEvent(_recEvent)
{}

inline AudioDevice::~AudioDevice()
{ recEvent.setInactive(); }

inline unsigned int AudioDevice::getSampleRate()
{ return (sampleRate); }

#endif

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

#include <iostream>

#include "jackAudioDevice.h"

int step;

int process (jack_nframes_t nframes, void *arg)
{
  jack_default_audio_sample_t *in, *out;

  JackAudioDevice* jad = (JackAudioDevice*)(arg);
  
  // std::cerr<<"jack process ";

  // only wanted to know about the current transport status
  //  jack_transport_state_t ts = jack_transport_query(jad->getClientHandle(), NULL);
  //if (ts == JackTransportRolling) {
    
    //   change inner state - is process called when there is no connection
    if (jad->isState(JackAudioDevice::Init))
      jad->changeState(JackAudioDevice::Idle);
    
    if (jad->isState(JackAudioDevice::RunRec) || jad->isState(JackAudioDevice::RunDuplex)){
      in = (jack_default_audio_sample_t*) jack_port_get_buffer (jad->inputPort, nframes);
      //      std::cerr<<"recording "<<nframes<<" frames ";
      if (jack_ringbuffer_write_space(jad->recRingBuffer) < sizeof(jack_default_audio_sample_t) * nframes)
	std::cerr <<" ERROR: ringbuffer buffer to small ";
      jack_ringbuffer_write(jad->recRingBuffer,(char*)in, 
			    sizeof(jack_default_audio_sample_t) * nframes);
    }

    if (jad->isState(JackAudioDevice::RunPlay) || jad->isState(JackAudioDevice::RunDuplex)){

      out = (jack_default_audio_sample_t*) jack_port_get_buffer (jad->outputPort, nframes);
      if ((jack_ringbuffer_read(jad->playRingBuffer,(char*)out,  
				sizeof(jack_default_audio_sample_t) * nframes)) < 
	  (sizeof(jack_default_audio_sample_t) * nframes))
	std::cerr << "jack process error not enough samples to play\n"; // 
    }
    //  }    
    //  std::cerr<<std::endl;
  //  if (ts == JackTransportStopped) 
  //    jad->changeState(JackAudioDevice::Exit);
  
  return 0;      
}


JackAudioDevice::JackAudioDevice(Scheduler* _scheduler, Event _recEvent)
  :AudioDevice(_scheduler, _recEvent)
{
  step = 0;
  // initialize the ring buffers
  recRingBuffer  = jack_ringbuffer_create(defaultRingBufferSize);
  playRingBuffer = jack_ringbuffer_create(defaultRingBufferSize);

  clientState = JackAudioDevice::Init;

  // get a client handle
  if (!(jackClientHandle = jack_client_new("jackAudioDevice"))) {
    std::cerr << "JackAudioDevice::JackAudioDevice: can not create jack client\n";
    exit(-1); // this is really a hard failure ..
  }
  
  // register "C" callback
  jack_set_process_callback (jackClientHandle, process, this);
  
  // TBD: shutdown!! 

  // whats our samplerate
  sampleRate = jack_get_sample_rate (jackClientHandle);

  // create the application (?) ports:
  inputPort = 
    jack_port_register (jackClientHandle, "input",
			JACK_DEFAULT_AUDIO_TYPE,
			JackPortIsInput, 0);

  outputPort = 
    jack_port_register (jackClientHandle, "output",
			JACK_DEFAULT_AUDIO_TYPE,
			JackPortIsOutput, 0);
  
  if ((inputPort == NULL) || (outputPort == NULL)) {
    std::cerr<< "no more JACK ports available\n";
    exit (1);
  }

  // decided to use char handles for the connection:
  appCapturePort = new char[jack_port_name_size()];
  strcpy(appCapturePort,jack_port_name(inputPort));

  appOutputPort = new char[jack_port_name_size()];
  strcpy(appOutputPort,jack_port_name(outputPort));

  // request the physical (?) ports:
  const char **ports = jack_get_ports (jackClientHandle, NULL, NULL,
                                JackPortIsPhysical|JackPortIsOutput);
  if (ports == NULL) {
    std::cerr<< "no physical capture ports\n";
    exit (1);
  }

  // hopefully no buffer overflows
  phyCapturePort = new char[jack_port_name_size()];
  strcpy(phyCapturePort,ports[0]);
  
  std::cerr << "JackAudioDevice(Constructor): physical Capture Port is "<<phyCapturePort<<"\n";

  free(ports);

  ports = jack_get_ports (jackClientHandle, NULL, NULL,
                                JackPortIsPhysical|JackPortIsInput);
  if (ports == NULL) {
    std::cerr<< "no physical capture ports\n";
    exit (1);
  }

  // hopefully no buffer overflows
  phyOutputPort = new char[jack_port_name_size()];
  strcpy(phyOutputPort,ports[0]);

  std::cerr << "JackAudioDevice(Constructor): physical Output Port is "<<phyOutputPort<<"\n";

  free(ports);

  if (jack_activate (jackClientHandle)) {
    std::cerr<<"cannot activate jackClientHandle";
    exit (1);
  }

}

JackAudioDevice::~JackAudioDevice()
{
  // disconnect play
  if ((isState(JackAudioDevice::RunDuplex)) || (isState(JackAudioDevice::RunPlay))) {
    jack_disconnect(jackClientHandle, phyOutputPort, appOutputPort);
  }
  
  // disconnect rec
  if ((isState(JackAudioDevice::RunDuplex)) || (isState(JackAudioDevice::RunRec))) {
    jack_disconnect(jackClientHandle, phyCapturePort, appCapturePort);
  }

  delete phyCapturePort;
  delete phyOutputPort;
  delete appCapturePort;
  delete appOutputPort;

  jack_ringbuffer_free(recRingBuffer);
  jack_ringbuffer_free(playRingBuffer);

  jack_client_close (jackClientHandle);

}

bool JackAudioDevice::isState(ClientState_t state)
{
  return (state == clientState);
}

JackAudioDevice::ClientState_t JackAudioDevice::getState()
{
  return (clientState);
}

jack_client_t* JackAudioDevice::getClientHandle()
{
  return (jackClientHandle);
}

bool JackAudioDevice::changeState(ClientState_t state)
{
  // TBD: if invalid transition -> return false
  clientState = state;
  return(true);
}

bool JackAudioDevice::openAudioPlayDevice()
{
  if (isState(JackAudioDevice::RunPlay) || isState(JackAudioDevice::RunDuplex))
    return (false);

  jack_connect(jackClientHandle, appOutputPort, phyOutputPort);

  if (isState(JackAudioDevice::RunRec))
    changeState(JackAudioDevice::RunDuplex);
  else
    changeState(JackAudioDevice::RunPlay);

  return(true);
}

void JackAudioDevice::closeAudioPlayDevice()
{
  jack_disconnect(jackClientHandle, appOutputPort, phyOutputPort);  
  jack_ringbuffer_reset(playRingBuffer);

  if (isState(JackAudioDevice::RunDuplex))
    changeState(JackAudioDevice::RunRec);
  else
    changeState(JackAudioDevice::Idle);

}

bool JackAudioDevice::openAudioRecDevice()
{
  if (isState(JackAudioDevice::RunRec) || isState(JackAudioDevice::RunDuplex))
    return (false);

  std::cerr << "JackAudioDevice::openAudioRecDevice: connecting "<<phyCapturePort<<" and "<<appCapturePort<<std::endl;

  jack_connect(jackClientHandle, phyCapturePort, appCapturePort);

  std::cerr << "state is :"<<clientState<<std::endl;

  if (isState(JackAudioDevice::RunPlay))
    changeState(JackAudioDevice::RunDuplex);
  else
    changeState(JackAudioDevice::RunRec);

  std::cerr << "state is :"<<clientState<<std::endl;

  // set timeout for the first portion of data
  ownRecEvent = Event(0,getTics()+recCallInterval);
  setEvent(ownRecEvent);

  return(true);
}

void JackAudioDevice::closeAudioRecDevice()
{
  jack_disconnect(jackClientHandle, phyCapturePort, appCapturePort);  
  jack_ringbuffer_reset(recRingBuffer);

  if (isState(JackAudioDevice::RunDuplex))
    changeState(JackAudioDevice::RunPlay);
  else
    changeState(JackAudioDevice::Idle);

  ownRecEvent.setInactive();
}

bool JackAudioDevice::fillPlayBuffer(float* data, uint16 length)
{
  if (isState(JackAudioDevice::RunPlay) || isState(JackAudioDevice::RunDuplex)) {
    jack_ringbuffer_write(playRingBuffer, (char*)data, length*(sizeof(float)));
    return (true);
  }
  return (false);
}

bool JackAudioDevice::receiveRecording(float* data, uint16& length)
{
  if (isState(JackAudioDevice::RunRec) || isState(JackAudioDevice::RunDuplex)) {
    length = jack_ringbuffer_read(recRingBuffer, (char*) data, length*(sizeof(float)));
    length /= (sizeof(float));
    return(true);
  }
  return(false);
} 

void JackAudioDevice::eventHandler(Event event)
{
  if (event == ownRecEvent) {
    // call the parent process, that there are samples available
    if (receiveLength()) {
      Event recClone = recEvent.clone();
      //      recClone.setTimeout(getTics()+0.15*HZ);
      setEvent(recClone);
      //      std::cerr << "J";
    }
    ownRecEvent = Event(0,ownRecEvent.getTimeout()+recCallInterval);
    setEvent(ownRecEvent);
  }
}

uint16 JackAudioDevice::receiveLength()
{
  return (jack_ringbuffer_read_space(recRingBuffer)/(sizeof(float)));
}

/*
 * the web recorder backend - main backend application
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

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "webrecBackend.h"
#include "v4lCapture.h"
#include "v4l2Capture.h"
#include "scheduler.h"
#include "videoDisplayMsg.h"
#include "webrecMessage.h"
#include "jackAudioDevice.h"
#include "theoraCodec.h"
#include "simpleConnection.h"
#include "qThreadConnection.h"

#include "oggHeader.h"

WebrecBackend::WebrecBackend()
  : preview(false),
    play(false),
    record(false),
    filewrite(false),
    stream(false),
    guiConnection(0),
    videoConnection(0),
    codecConnection(0),
    codec(0),
    videoCapture(0),
    audioDevice(0),
    tmpFrame (0),
    videoCaptureType(WebrecBackend::unknown),
    capture_format(PixmapFormat::PIXFMT_YUV420P, 320, 240),
    dpy_fmt_id(PixmapFormat::PIXFMT_NONE),
    winWidth(320),
    winHeight(240),
    captureDesc(0)
{
  Event recCallEvent(WebrecBackend::audioCaptureRecv, 0 , this);
  audioDevice = new JackAudioDevice(scheduler, recCallEvent);

  codecInfo = new CodecInfo();

  codecInfo->audioSampleFrequency        = audioDevice->getSampleRate();
  codecInfo->audioBitrate                = 8000;
  codecInfo->videoWidth                  = 320;
  codecInfo->videoHeight                 = 240;
  codecInfo->videoFrequencyNumerator     = 1;
  codecInfo->videoFrequencyDenuminator   = 16;
  codecInfo->videoAspectRatioNumerator   = 1;
  codecInfo->videoAspectRatioDenuminator = 1;
  codecInfo->videoBitrate                = 128000;
  codecInfo->withAudio                   = true;

  initCodec();
  //initStreamer("127.0.0.1", 12001);
}

WebrecBackend::~WebrecBackend()
{}

bool WebrecBackend::startCaptureDevice()
{
  if (videoCapture) {
    // we are actually captureing, so do nothing
    return (true);
  }

  if (videoCaptureType == WebrecBackend::unknown) {
    videoCapture = new v4l2Capture();
    if (videoCapture->open("/dev/video0") == -1)
      {
	delete videoCapture;
	videoCapture = new v4lCapture();
	if (videoCapture->open("/dev/video0") == -1)
	  {
	    fprintf(stderr,"can not handeln capture \n");
	    delete videoCapture; videoCapture = 0;
	    return false;
	  }
	else // openvideo via v4l was successfull
	  videoCaptureType = WebrecBackend::v4l;
      }
    else // open video via v4l2 was successfull
      videoCaptureType = WebrecBackend::v4l2;
  }
  else {
    if (videoCaptureType == WebrecBackend::v4l)
      videoCapture = new v4lCapture();
    if (videoCaptureType == WebrecBackend::v4l2)
      videoCapture = new v4l2Capture();
    videoCapture->open("/dev/video0");
  }

  if (videoCaptureType == WebrecBackend::unknown) {
    std::cerr << "WebrecBackend::startCaptureDevice: no webcam device found for\n";
  }

  if (videoCaptureType == WebrecBackend::v4l)
    std::cerr << "WebrecBackend::startCaptureDevice: using video4linux webcam driver\n";
  else
    std::cerr << "WebrecBackend::startCaptureDevice: using video4linux2 webcam driver\n";

      

  //  capture_format = PixmapFormat(PixmapFormat::PIXFMT_YUV420P, 320, 240);
  
  if( videoCapture->setformat(capture_format) != 0) {
      fprintf(stderr,"capture format not supported \n");
      capture_format.print();
      return false;
    }
  capture_format.print();
  
  if ((captureDesc = videoCapture->startvideo(-1,2)) == -1) {
    return false;
  }

  std::cerr<<"captureDesc ="<< captureDesc<<std::endl;
  scheduler->connectDescriptor(this, captureDesc, videoCaptureRecv);
  
  std::cerr << "WebrecBackend::startCaptureDevice: opening audio capture device\n";
  audioDevice->openAudioRecDevice();
  audioDevice->openAudioPlayDevice();
  
  return true;
}

void WebrecBackend::stopCaptureDevice()
{
  std::cerr<<"WebrecBackend::stopCaptureDevice: closing audio capture device\n";
  audioDevice->closeAudioRecDevice();
  audioDevice->closeAudioPlayDevice();

  videoCBREvent.setInactive();

  if (captureDesc) {
    std::cerr << "WebrecBackend::stopCaptureDevice: stop video capture\n";
    videoCapture->stopvideo();
    videoCapture->close();
    scheduler->disconnectDescriptor(captureDesc);
    captureDesc = 0;

    // TBD: maybe this could be done more elegant, but if I leave the 
    // videoCapture device the mmap is not working correct (is it deleted)
    // the second time the capture device has been opened, so:
    
    delete videoCapture;
    videoCapture = 0;
  }
}

void WebrecBackend::eventHandler(Event event)
{
  Message* msg = 0;
  //  std::cerr << "WebrecBackend::eventHandler: event <"<<event.getID()<<"> raised\n";
  switch(event.getID()) {

  case guiMsgRecv: {
    guiConnection->receiveMessage(msg, this);
    handleGuiMsg(msg);
  } break;

  case videoMsgRecv: {
    videoConnection->receiveMessage(msg, this);  
    handleVideoMsg(msg);
  } break;

  case videoCaptureRecv: {
    handleVideoCapture();
  } break;

  case audioCaptureRecv: {
    handleAudioCapture();
  } break;

  case videoCFRTimeout: {
    handleCFRVideoCapture();
  } break;

  case codecMsgRecv: {
    codecConnection->receiveMessage(msg,this);
    handleCodecMessage(msg);
  } break;

  default: break;

  };
  delete msg;
}

void WebrecBackend::handleGuiMsg(Message* message)
{
  WebrecMessage* msg = static_cast<WebrecMessage*>(message);

  switch (message->getID()) {
  
  case WebrecMessage::play: {
    std::cerr<<"WebrecBackend::handleGuiMsg: handle play message\n";
  } break;
  
  case WebrecMessage::record: {
    if (!record) {
      if (!preview)
	startCaptureDevice();

      // if we want to stream start streamer
      initStreamer(streamName); // streamnik server

      setEvent(videoCBREvent = Event(videoCFRTimeout, getTics()+videoCBRTimeoutTics));
      StartEncoderMessage* encMsg = new StartEncoderMessage(codecInfo);
      codecConnection->sendMessage(encMsg,this);
      // open file

      // send some sound samples ..
      float waitSec = 0.1;
      float buffer[(int)(16000*waitSec)];
      for (unsigned int i= 0; i<16000*waitSec; i++)
	buffer[i]=0;
      CodecDataMessage* msg = new CodecDataMessage(CodecMessage::rawAudioData,
						   (char*)buffer, 
						   (int)(16000*waitSec)*sizeof(float));
      codecConnection->sendMessage(msg,this);

      if (fileName.empty()) {
	filewrite = false;
	fileDesc = 0;
      }
      else {
	if ((fileDesc = fopen(fileName.c_str(),"wb"))>0)
	  filewrite = true;
      }
      record = true; 
    }
    else {
      if (!preview)
	stopCaptureDevice();
      videoCBREvent.setInactive();
      StopEncoderMessage* encMsg = new StopEncoderMessage();
      codecConnection->sendMessage(encMsg,this);
      //      record = false;
    }
    std::cerr<<"WebrecBackend::handleGuiMsg: handle record message\n";
    
  } break;
  
  case WebrecMessage::preview: {
    std::cerr<<"WebrecBackend::handleGuiMsg: handle preview message\n";
    if (msg->trType == WebrecMessage::BackendSet) {
      preview = true;
      startCaptureDevice();
    }
    if (msg->trType == WebrecMessage::BackendUnset) {
      preview = false;
      stopCaptureDevice();
    }
  } break;
  
  case WebrecMessage::filename: {
    std::cerr<<"WebrecBackend::handleGuiMsg: handle filename message <";
    fileName = ((FileInfoMessage*)message)->filename;
    std::cerr<<fileName<<">\n";
  } break;

  case WebrecMessage::streamname: {
    std::cerr<<"WebrecBackend::handleGuiMsg: handle streamname message <";
    streamName = ((StreamInfoMessage*)message)->streamname;
    std::cerr<<streamName<<">\n";
  } break;
  
  case WebrecMessage::codecInfo: {
    std::cerr<<"WebrecBackend::handleGuiMsg: handle codecInfo message\n";
    //    CodecInfoMessage* m = static_cast<CodecInfoMessage*>(msg);
    /*
    switch (m->mediaType) {
    case CodecInfoMessage::audio: {
      encoderConfig.audioEncoderBitrate = m->codecBitrate;
      encoderConfig.audioEncoderID = m->codecType;
    } break;
    case CodecInfoMessage::video: {
      encoderConfig.videoEncoderBitrate = m->codecBitrate;
      encoderConfig.videoEncoderID = m->codecType;
    } break;
  }
    */
  } break;
  
 default:
    std::cerr<<"WebrecBackend::handleGuiMsg: unknown message ID = "<<msg->getID()<<std::endl;
  }

  return;
}

void WebrecBackend::handleVideoCapture()
{

  if (captureDesc == 0) {
    fprintf(stderr,"video capture invalid state - returning \n");
    return;
  }

  // we need a constant picture/frame rate, so we only store 
  // the captured frame. If the capture is slower than the framerate,
  // there will be two of the same frames be converted,
  // if it is faster, some frames will be unrecognized.

  //  std::cerr<<name<<": gathering captured frame ";

  if (tmpFrame) {
    delete tmpFrame->data_ptr;
    delete tmpFrame;
    tmpFrame = 0;
  }

  tmpFrame = videoCapture->nextframe();

  //  std::cerr << "c";

  // gonna creating a preview for the viewer
  if (preview || record) {
    VideoFrame* preview_frame = converter.convert(tmpFrame,dpy_fmt_id);
    putFrame(preview_frame);
  }

  return;
}

void WebrecBackend::handleCFRVideoCapture()
{
  // method will be called in constant time, to create 
  // constant framerate
  setEvent(videoCBREvent = Event(videoCFRTimeout, videoCBREvent.getTimeout()+videoCBRTimeoutTics));

  if (!tmpFrame) {
    fprintf(stderr,"video capture failed - returning \n");
    return;
  }

  //  std::cerr << "C";

  CodecDataMessage* msg = new CodecDataMessage(CodecMessage::rawVideoData,
					       (char*)tmpFrame->data(), 
					       winWidth*winHeight*3/2);
  codecConnection->sendMessage(msg,this);
  
  return;
}

void WebrecBackend::handleAudioCapture()
{
  uint16 length;
  if (!(length = audioDevice->receiveLength()))
    return;

  float data[length];
  if (audioDevice->receiveRecording(data, length)) {
    //    std::cerr<<".";
    audioDevice->fillPlayBuffer(data,length);
    if (!record)
      return;

    CodecDataMessage* msg = new CodecDataMessage(CodecMessage::rawAudioData,
						 (char*)data, 
						 length*sizeof(float));
    codecConnection->sendMessage(msg,this);
  }
}

void WebrecBackend::putFrame(VideoFrame* frame)
{
  if (videoConnection) {
    VideoDisplayMsg::PictureFrame* msg = new VideoDisplayMsg::PictureFrame(frame);
    videoConnection->sendMessage(msg, this);
  }
  else {
    delete frame->data_ptr;
    delete frame;
  }
}


// this is video DISPLAY message
void WebrecBackend::handleVideoMsg(Message* message)
{
  switch (message->getID()) {

  case VideoDisplayMsg::type_dpy: {
    VideoDisplayMsg::Dpy* dpyMsg = static_cast<VideoDisplayMsg::Dpy*>(message);
    dpy_fmt_id = dpyMsg->fmt_id;
    fprintf(stderr,"dpy_format.id() = %d \n", dpy_fmt_id);
    //    coder = new AVCodecHandler();
    //    coder->init(encoderConfig, dpy_fmt_id);
      //    coder->init_decoder(dpy_fmt_id);
  } break;

  case VideoDisplayMsg::type_resize: {
    VideoDisplayMsg::Resize* resizeMsg = static_cast<VideoDisplayMsg::Resize*>(message);
    winWidth  = resizeMsg->width;
    winHeight = resizeMsg->height;
  } break;

  default:
    std::cerr<<"WebrecBackend::handleVideoMsg: unknown message ID="<<message->getID()<<std::endl; 
  }

  return;
}


void WebrecBackend::setGUIConnection(Connection& _transfer)
{
  guiConnection = &_transfer;
  guiConnection->addTask(this, guiMsgRecv, scheduler);
}

void WebrecBackend::setVideoConnection(Connection& _transfer)
{
  videoConnection = &_transfer;
  videoConnection->addTask(this, videoMsgRecv, scheduler);
}

void WebrecBackend::initCodec()
{
  codecConnection = new QThreadConnection(scheduler);
  codecConnection->addTask(this, codecMsgRecv, scheduler);

  // this will be more generic in further version
  codec = new TheoraCodec;

  if (!codec->connectDecoderPipe(*codecConnection)) {
    std::cerr<<"WebrecBackend::initCodec: ERROR could not initialize codec\n";
    delete codec;
    codec=0;
    return;
  }

  //  codec->startEncoder(codecInfo);

  // start the codec thread
  codec->start(); 
}

void WebrecBackend::initStreamer(std::string streamname)
{
  if (streamname.empty()) {
    stream = false;
    return;
  }

  int port = 12002; // standard port

  // do we have an explicite port?
  std::string::size_type dpPos = std::string::npos;
  std::string::size_type slPos = std::string::npos; // position of : and /
  std::string::size_type urlLast = streamname.length();

  if ((dpPos = streamname.find_last_of(":")) != std::string::npos) {
    urlLast = dpPos;
    if ((slPos = streamname.find("/",dpPos)) == std::string::npos)
      slPos = streamname.length();
    else 
      slPos -= 1;
    if (slPos > dpPos+1) {
      port = atoi(streamname.substr(dpPos+1,slPos-dpPos).c_str());
    }    
  }

  std::string::size_type urlPos;
  if ((urlPos = streamname.find("://")) != std::string::npos) {
    std::cerr << "streaming to protocol <"<<streamname.substr(0,urlPos-1)<<"> not supported\n";
    return;
  } else
    urlPos=0;

  std::string url = streamname.substr(urlPos,urlLast-urlPos);

  struct hostent *h;

  if ((h=gethostbyname(url.c_str())) == NULL) {  // get the host info
    herror("gethostbyname");
    return;
  }

  std::cerr << "streaming to url "<<url<<" ("<<inet_ntoa(*((in_addr *)h->h_addr))<<")  port "<< port<<"\n"; 

  struct sockaddr_in streamAddr;
  
  if ((streamDesc = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
    perror("socket");
    stream = false;
    return;
  }

  streamAddr.sin_family = AF_INET;    // host byte order 
  streamAddr.sin_port = htons(port);  // short, network byte order 
  streamAddr.sin_addr = *((in_addr*)(h->h_addr));
  memset(&(streamAddr.sin_zero), '\0', 8);  // zero the rest of the struct 

  if (connect(streamDesc, (struct sockaddr *)&streamAddr,
	      sizeof(struct sockaddr)) == -1) {
    std::cerr << "WebrecBackend::initStreamer: could not establish stream sender\n";
    perror("connect");
    stream = false;
    return;
  }

  stream = true;
}

void WebrecBackend::handleCodecMessage(Message* message)
{
  static int counter = 0;
  //  std::cerr <<"§";
  switch (message->getID()) {

  case CodecMessage::decodedData: {
    //abort();
    /*
    std::cerr<<"\n\ncounter= "<<counter<<std::endl;
    counter++; // testing robustness
    if (counter < 50)
      break;
    */    
    CodecDataMessage* codecMessage = static_cast<CodecDataMessage*>(message);

    // run through the data and find a header

    /*
    for (unsigned int i=0; i<codecMessage->length;++i) {
      if (strncmp(codecMessage->data+i, "OggS", 4)==0) {

	oggHeader* header = (oggHeader*)(codecMessage->data+i);

	std::cerr << header->ogg <<" ";
	
	if (header->pack_type)
	  std::cerr << "fresh packet | ";
	else
	  std::cerr << "continued packet | ";
    
	if (header->page_type)
	  std::cerr << "first page (bos) | ";
	else 
	  std::cerr << "not first | ";
	
	if (header->last)
	  std::cerr << "last page | ";
	else
	  std::cerr << "not last page | ";
	
	std::cerr << " pos: "<<header->position;
	std::cerr << " serial: "<<header->serial;
	std::cerr << " pageNo: "<<header->pageNo;
	std::cerr << " checksum: "<<header->checksum<<std::endl;
      }
    }
*/
    
    if (filewrite)
      fwrite(codecMessage->data, 1, codecMessage->length, fileDesc);
    if (stream) {
      unsigned int dataLength = codecMessage->length;
      //TBD Error Handling
      //      std::cerr << "len: "<<dataLength<<"\n";
    

      send(streamDesc,&dataLength, sizeof(unsigned int), 0);
      send(streamDesc,codecMessage->data, codecMessage->length, 0);
    }
  } break;
    
  case CodecMessage::decodedHeader: {
    CodecDataMessage* codecMessage = static_cast<CodecDataMessage*>(message);

    // run through the data and find a header

    /*
    for (unsigned int i=0; i<codecMessage->length;++i) {
      if (strncmp(codecMessage->data+i, "OggS", 4) == 0) {

	oggHeader* header = (oggHeader*)(codecMessage->data+i);

	std::cerr << header->ogg <<" ";
	
	if (header->pack_type)
	  std::cerr << "fresh packet | ";
	else
	  std::cerr << "continued packet | ";
    
	if (header->page_type)
	  std::cerr << "first page (bos) | ";
	else 
	  std::cerr << "not first | ";
	
	if (header->last)
	  std::cerr << "last page | ";
	else
	  std::cerr << "not last page | ";
	
	std::cerr << " pos: "<<header->position;
	std::cerr << " serial: "<<header->serial;
	std::cerr << " pageNo: "<<header->pageNo;
	std::cerr << " checksum: "<<header->checksum<<std::endl;
      }
    }
    */
    if (filewrite) {
      fwrite(codecMessage->data, 1, codecMessage->length, fileDesc);
    }
    /*
    if (stream) {
      unsigned int dataLength = codecMessage->length;
      //TBD Error Handling
      std::cerr << "len: "<<dataLength<<"\n";
            send(streamDesc,&dataLength, sizeof(unsigned int), 0);
      send(streamDesc,codecMessage->data, codecMessage->length, 0);
      
    }
    */
    //    counter += codecMessage->length;
    // actually we do not write the header to a stream
  } break;

  case CodecMessage::stopEncoder: {
    if (filewrite)
      fclose(fileDesc);
    if (stream)
      ::close(streamDesc);
    record = false;
  } break;

  }

}


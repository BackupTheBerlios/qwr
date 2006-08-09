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

#ifndef webrecBackend_h
#define webrecBackend_h

#ifdef WITH_QT
#include "qTask.h"
#else
#include "itask.h"
#endif

#include "videoCapture.h"
#include "codecInterface.h"
#include "audioDevice.h"
#include "convert.h"

//! Webrecorder Backend Class
/*! This class will provide:
 -# GUI communication
 -# communication to the X-Server for the display
 -# communication to the capture device (webcam)

What should happen here is: 
creating a preview on request, creating a file on request and
stream to that file or streaming the data to a server/multiplexer.
*/
#ifdef WITH_QT
class WebrecBackend : public QTask {
#else
  class WebrecBackend : public ITask {
#endif

 protected:
  enum VideoCaptureType {
    unknown,
    v4l,
    v4l2
  };

  bool preview;
  bool play;
  bool record;

  bool filewrite;
  bool stream;

  // connections through which we get information
  Connection* guiConnection;
  Connection* videoConnection;
  Connection* codecConnection;

  // Codec Objects:
  CodecInfo* codecInfo;
  CodecInterface* codec; 

  // converter
  Convert converter;

  // Capture Objects
  VideoCapture* videoCapture;
  AudioDevice* audioDevice;

  struct VideoFrame* tmpFrame; // needed to get constant bit rate pictured

  VideoCaptureType videoCaptureType;

  PixmapFormat                capture_format;
  PixmapFormat::PixelFormat   dpy_fmt_id;

  /* display window sizes */
  int                         winWidth;
  int                         winHeight;
  
  int                         frameSeqNo;
    
  int                         captureDesc;
  
  Event                       videoCBREvent;

  FILE*                       fileDesc;
  int                         streamDesc;
  std::string                 fileName;
  std::string                 streamName;

  static const int videoCBRTimeoutTics = (int)(1.0/16.0*HZ);

  bool setCaptureFormat(/* int w, int h, int fmtid */);
  
  bool startCaptureDevice();
  void stopCaptureDevice();

  void handleGuiMsg(Message* message);
  void handleVideoMsg(Message* message);
  void handleVideoCapture();
  void handleAudioCapture();
  void handleCFRVideoCapture();
  void handleCodecMessage(Message* message);

  void putFrame(VideoFrame* frame);

  void initCodec();
  void initStreamer(std::string streamname); 

 public:
  enum eventID {
    timeout          = 0,
    guiMsgRecv       = 1,
    videoMsgRecv     = 2,
    videoCaptureRecv = 3,
    audioCaptureRecv = 4,
    videoCFRTimeout  = 5,
    codecMsgRecv     = 6
  };

  WebrecBackend();
  virtual ~WebrecBackend();

  virtual void eventHandler(Event event);

  void setGUIConnection(Connection& _transfer);
  void setVideoConnection(Connection& _transfer);

};

#endif

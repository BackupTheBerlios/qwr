/*
 * webcam recorder main class
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

#include <iostream>

#include <qsocketnotifier.h>
#include <qradiobutton.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qcombobox.h>

#include "webrec.h"
#include "webrecMessage.h"
#include "videowidget.h"

WebRec::WebRec()
  :qtWebRec()
{
  // usually event ID should be specified by the backend
  backend.setGUIConnection(guiTransfer);
  //  backend.init();
  backend.start();

  backend.setVideoConnection(videoTransfer);

  connectNotifier(guiTransfer.openExternalConnection());
  videoWidget1->setConnector(&videoTransfer);

  // initialize the video codecs
  initVideoCodecDisplay();
  initAudioCodecDisplay();

}

void WebRec::initVideoCodecDisplay()
{
  // filling the record
  videoCodecRecord.push_back(CodecRecord(0,"Theora"));
  /*
  videoCodecRecord.push_back(CodecRecord(CODEC_ID_MSMPEG4V3,"MS MPEG 2"));
  videoCodecRecord.push_back(CodecRecord(CODEC_ID_MPEG4,"MPEG 4"));
  videoCodecRecord.push_back(CodecRecord(CODEC_ID_RV40,"Realvideo 4"));
  */
  // inserting record to Display combobox
  vidCodecCombo->clear();
  for (unsigned int i=0; i<videoCodecRecord.size(); ++i)
    vidCodecCombo->insertItem(videoCodecRecord[i].codecName,i);

}

void WebRec::initAudioCodecDisplay() 
{
  // TBD
}

WebRec::~WebRec()
{
}

void WebRec::connectNotifier(int socket)
{
  sockNotifier = new QSocketNotifier (socket,QSocketNotifier::Read);
  connect( sockNotifier , SIGNAL(activated(int)), this, SLOT(handleBackendEvent(int)));
}


void WebRec::play(bool togglePlay)
{
  std::cerr << "WebRec::play: calling Backend\n";
  if (togglePlay)
    //    preview->setOn(false);
    guiTransfer.sendMessage(new WebrecMessage(WebrecMessage::play, WebrecMessage::BackendSet));
  else
    guiTransfer.sendMessage(new WebrecMessage(WebrecMessage::play, WebrecMessage::BackendUnset));
}

void WebRec::record(bool toggleRec)
{
  std::cerr << "WebRec::record: calling Backend\n";
  if (toggleRec) 
    guiTransfer.sendMessage(new WebrecMessage(WebrecMessage::record, WebrecMessage::BackendSet));
  else
    guiTransfer.sendMessage(new WebrecMessage(WebrecMessage::record, WebrecMessage::BackendUnset));
}

void WebRec::stream(bool toggle)
{
  std::cerr << "WebRec::record: calling Backend\n";
  if (toggle) {
    std::string streamname = streamurl->text();
    guiTransfer.sendMessage(new StreamInfoMessage(WebrecMessage::BackendSet, streamname));
    //    guiTransfer.sendMessage(new WebrecMessage(WebrecMessage::stream, WebrecMessage::BackendSet));
  }
  else
    guiTransfer.sendMessage(new StreamInfoMessage(WebrecMessage::BackendSet, std::string("")));
    //    guiTransfer.sendMessage(new WebrecMessage(WebrecMessage::stream, WebrecMessage::BackendUnset));
}


void WebRec::selectFile(const QString& filename)
{
  std::cerr << "Webrec::selectFile: calling Backend <"<<filename<<">\n";
  guiTransfer.sendMessage(new FileInfoMessage(WebrecMessage::BackendSet, filename));
}

void WebRec::selectStream(const QString& streamname)
{
  std::cerr << "Webrec::selectStream: calling Backend <"<<streamname<<">\n";
  guiTransfer.sendMessage(new StreamInfoMessage(WebrecMessage::BackendSet, streamname));
}

void WebRec::videoSelect(int value)
{
  std::cerr << "Webrec::videoSelect: calling Backend ID: "<<value<<"\n";
  guiTransfer.sendMessage(new CodecInfoMessage(WebrecMessage::BackendSet, 
					       CodecInfoMessage::video,
					       videoCodecRecord[value].codecType,
					       128000));
}

void WebRec::togglePreview(bool togglePreview)
{
  std::cerr << "WebRec::togglePreview: calling Backend\n";
  if (togglePreview)
    guiTransfer.sendMessage(new WebrecMessage(WebrecMessage::preview, WebrecMessage::BackendSet));
  else
    guiTransfer.sendMessage(new WebrecMessage(WebrecMessage::preview, WebrecMessage::BackendUnset));
}

void WebRec::audioSelect(int value)
{
  // TBD
}

void WebRec::handleBackendEvent(int recvSocket)
{
  Message* msg;
  guiTransfer.receiveMessage(msg);
  //  CountMessage* message = static_cast<CountMessage*>(msg);

  std::cerr << "WebRec::readFromSocket: message received \n";

  // switch for the message ID

}

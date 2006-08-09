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

#ifndef webrec_h
#define webrec_h

#include "qtwebrec.h"
#include "qDescConnection.h"
#include "webrecBackend.h"

class QSocketNotifier;

class CodecRecord {

 public:
  std::string codecName;
  int         codecType;

  CodecRecord(int _codecType, const std::string& _codecName)
    : codecName(_codecName), codecType(_codecType) {}

};

class WebRec : public qtWebRec {

  Q_OBJECT
    
  protected:

  std::vector<CodecRecord> videoCodecRecord;
  std::vector<CodecRecord> audioCodecRecord;

  public:
  WebRec();
  virtual ~WebRec();
  
  public slots:
  virtual void play(bool );
  virtual void record(bool );
  virtual void stream(bool );
  virtual void selectFile(const QString& );
  virtual void videoSelect(int);
  virtual void togglePreview(bool);
  virtual void audioSelect(int);
  virtual void selectStream(const QString& );

  virtual void handleBackendEvent(int);

 protected:
  QSocketNotifier*  sockNotifier;
  QDescConnection   guiTransfer;
  QDescConnection   videoTransfer;
  WebrecBackend     backend;
  
  void connectNotifier(int socket);
  void initVideoCodecDisplay();
  void initAudioCodecDisplay();
};

#endif

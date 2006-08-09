/*
 * descriptor connection between a task and a pipe
 * Copyright (c) 2005-2006 Joern Seger
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef descConnection_h
#define descConnection_h

#include "connection.h"

#include "cc++/thread.h"

class DescConnection : public Connection {

 protected:

  class PipeData {
  public:
    int desc[2];
  };

  PipeData pipeInfo[2];

  ost::Mutex Lock[2];
  
  virtual void inform(unsigned char taskID);
  virtual uint32 receive(unsigned char taskID);

  virtual void lockTaskQueue(unsigned char taskID); 
  virtual void unlockTaskQueue(unsigned char taskID);

  virtual void _addTask(unsigned char taskID);
  virtual void _delTask(unsigned char taskID);

 public:
  DescConnection(Scheduler* scheduler = 0);
  virtual ~DescConnection();

  int openExternalConnection();
  void sendMessage(Message* message);
  void receiveMessage(Message* &message);

};

#endif

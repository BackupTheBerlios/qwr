/*
 * connection between two tasks; not threadsave
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

#ifndef simpleConnection_h
#define simpleConnection_h

#include "connection.h"
#include "scheduler.h"

class SimpleConnection : public Connection {

 protected:
  Scheduler* scheduler; 

  virtual void   inform(unsigned char TaskID);
  virtual uint32 receive(unsigned char taskID);

  virtual void lockTaskQueue(unsigned char TaskID); 
  virtual void unlockTaskQueue(unsigned char TaskID);

  virtual void _addTask(unsigned char taskID);
  virtual void _delTask(unsigned char taskID);
  
 public:
  SimpleConnection(Scheduler* scheduler);
  virtual ~SimpleConnection();

};

#endif

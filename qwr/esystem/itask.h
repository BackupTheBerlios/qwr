/*
 * threaded task, running in its own context
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

#ifndef itask_h
#define itask_h

#include "task.h"
#include "threadConnection.h"
#include <cc++/thread.h>

class ITask : public Task, public ost::Thread {

 protected:
  virtual void run();

 public:
  ITask();
  virtual ~ITask();

  virtual void init(){};
  virtual void eventHandler(Event event) = 0;

  //! add a connection to this task
  /*! must take place befor the thread started 
   Connection should be a multithread-able connection */
  void addConnection(Connection& transfer, uint32 eventID);

};

#endif

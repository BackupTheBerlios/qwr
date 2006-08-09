/*
 * abstract task
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

#ifndef task_h
#define task_h

class Scheduler;

#include <string>
#include "event.h"

class Task {

 private:
  uint32*        schedulerTime;

 protected:
  Scheduler*     scheduler;
  std::string    name;
  bool           isInitialized;

  void           setEvent(Event event);
  uint32         getTics();

 public:
  Task(Scheduler* scheduler);
  virtual ~Task();

  virtual void   init(){};

  void           setTaskName(const std::string& name);
  std::string&   getTaskName();

  virtual void   eventHandler(Event event) = 0;

};

#endif

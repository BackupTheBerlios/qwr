/*
 * main scheduler
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

#ifndef scheduler_h
#define scheduler_h

#include <map>

#include "prioQueue.h"
#include "definitions.h"
#include "task.h"
#include "timev.h"

class Scheduler {

 public:
  enum selectType_t {
    readSelect,
    writeSelect,
    errorSelect
  };

 protected:

  class TaskListSelectItem {
   
  public:
    Task*        task;
    uint32       eventID;
    TaskListSelectItem(Task* task=0, uint32 eventID=0);

  };

  static const uint32 maxSelectTimeWait = 2*HZ; 

  uint32 timeTics; // actual time tics
  
  timev startTime; // time when scheduling has started
  timev nextEvent; // time when next event will be scheduled

  std::map<int, TaskListSelectItem> TaskListSelectRead;
  std::map<int, TaskListSelectItem> TaskListSelectWrite;
  std::map<int, TaskListSelectItem> TaskListSelectError;

  // will be rewritten!!
  //  std::priority_queue<Event, std::vector<Event>, isEventLess> EventList;
  prioQueue EventList;
  
  void handleEvent(Event event);
  void setNextEvent();
  void setNowTics();

 public:

  Scheduler();
  virtual ~Scheduler();

  uint32* getTimePtr();

  void addEvent(Event event);

  void delEvents(Task* task);
  void delEvents(Task* task, unsigned int eventID);

  void connectDescriptor   (Task* task, int descriptor, uint32 eventID, 
			    selectType_t sType = readSelect);
  void disconnectDescriptor(int descriptor, selectType_t sType = readSelect);

  void schedule(uint32 time=0);

};

#endif

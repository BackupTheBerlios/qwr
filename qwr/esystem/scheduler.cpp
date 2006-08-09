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

#include <iostream>
#include "scheduler.h"

#include "timev.h"

Scheduler::TaskListSelectItem::TaskListSelectItem(Task* _task, uint32 _eventID)
  : task(_task), eventID(_eventID)
{}

Scheduler::Scheduler()
  : timeTics(0)
{
  nextEvent.set_infinite();
  startTime.set_infinite();
}

Scheduler::~Scheduler()
{
  
}

uint32* Scheduler::getTimePtr()
{
  return(&timeTics);
}

void Scheduler::addEvent(Event event)
{
  if (event.ptr()->task == 0)
    abort();
  EventList.push(event);
}

void Scheduler::connectDescriptor(Task* task, int descriptor, uint32 eventID,
				  selectType_t sType)
{
  switch (sType) {
  case Scheduler::readSelect:
    TaskListSelectRead[descriptor] = TaskListSelectItem(task, eventID);
    break;
  case Scheduler::writeSelect:
    TaskListSelectWrite[descriptor] = TaskListSelectItem(task, eventID);
    break;
  case Scheduler::errorSelect:
    TaskListSelectError[descriptor] = TaskListSelectItem(task, eventID);
    break;
  }
}

void Scheduler::disconnectDescriptor(int descriptor, selectType_t sType)
{
  switch (sType) {
  case Scheduler::readSelect:
    TaskListSelectRead.erase(descriptor);
    break;
  case Scheduler::writeSelect:
    TaskListSelectWrite.erase(descriptor);
    break;
  case Scheduler::errorSelect:
    TaskListSelectError.erase(descriptor);
    break;
  }    
}

void Scheduler::delEvents(Task* task)
{
  EventList.disableEvents(task);
}

void Scheduler::delEvents(Task* task, unsigned int eventID)
{
  EventList.disableEvents(task, eventID);
}

void Scheduler::handleEvent(Event event)
{
  if (event.isActive())
    event.ptr()->task->eventHandler(event);
}

void Scheduler::setNextEvent()
{
  // delete none active events
  while (!(EventList.empty()) && !(EventList.top().isActive()))
    EventList.pop();
  
  // if there is no open event, we only wait for descriptors
  if (EventList.empty()) {
    if ((TaskListSelectRead.empty()) &&
	(TaskListSelectWrite.empty()) &&
	(TaskListSelectError.empty()))
      exit(0); // we are done
    nextEvent.set2now();
    nextEvent += timev(maxSelectTimeWait);
  }
  else {
    unsigned int to = EventList.top().getTimeout();
    nextEvent = timev((unsigned int)((to/HZ)*1000 + (1000.0/HZ * (to%HZ))));
    nextEvent += startTime;
  }
}

void Scheduler::setNowTics()
{
  // what time has been gone up until now?
  timev now;
  timev dif;
  
  now.set2now();
  dif = now - startTime; 
  timeTics = (dif.tv_sec * HZ) + ((dif.tv_usec*HZ/MAXUSECPERSEC));  
}


void Scheduler::schedule(uint32 time)
{
  fd_set readfds, writefds, errorfds;
  int retval;
  int maxfd;
  std::map<int, TaskListSelectItem>::iterator TLSIterator;
  timev wait;

  uint32 maxTics = time*HZ;

  // if this is the first time, we get here, initialize the time
  if (nextEvent.infinite()) {
    startTime.set2now();
    nextEvent.set2now();
  }

  while((!maxTics) || (timeTics < maxTics)) {
    setNowTics();

    Event event;
    while ((!EventList.empty()) && (event = EventList.top()).isExpired(timeTics)) {
      EventList.pop();

#ifndef WITH_BADGUY_TEST

      handleEvent(event);
      
#else

      timev now;
      now.set2now();
      handleEvent(event);
      timev execTime;
      execTime.set2now();
      timev diff(20); // 20msec
      execTime -= now;
	if (execTime > diff)
	  std::cerr<<"Scheduler: slow task, difference is "
		   <<execTime.toString()<<"\n";
	
#endif
    }

    setNextEvent();

    while (!nextEvent.expired()) {
    
      // wait until a descriptor is set, the time is up
      FD_ZERO(&readfds);
      FD_ZERO(&writefds);
      FD_ZERO(&errorfds);
      maxfd = 0;

      for (TLSIterator=TaskListSelectRead.begin();
	   TLSIterator!=TaskListSelectRead.end();
	   ++TLSIterator){
	maxfd = ( (TLSIterator->first > maxfd) ? TLSIterator->first : maxfd);
	FD_SET(TLSIterator->first, &readfds);
      }

      for (TLSIterator=TaskListSelectWrite.begin();
	   TLSIterator!=TaskListSelectWrite.end();
	   ++TLSIterator){
	maxfd = ( (TLSIterator->first > maxfd) ? TLSIterator->first : maxfd);
	FD_SET(TLSIterator->first, &writefds);
      }

      for (TLSIterator=TaskListSelectError.begin();
	   TLSIterator!=TaskListSelectError.end();
	   ++TLSIterator){
	maxfd = ( (TLSIterator->first > maxfd) ? TLSIterator->first : maxfd);
	FD_SET(TLSIterator->first, &errorfds);
      }

      wait = nextEvent.expireTime();
      retval = select(maxfd+1, &readfds, &writefds, &errorfds, &wait);
    
      // if this is not a timeout find the connected ask and call it.
      if (retval > 0) {
	setNowTics();
	
	for (TLSIterator=TaskListSelectRead.begin(); 
	     TLSIterator!=TaskListSelectRead.end(); 
	     ++TLSIterator)
	  if (FD_ISSET(TLSIterator->first, &readfds)) {
	    Event event(TLSIterator->second.eventID, 
			timeTics, 
			TLSIterator->second.task);
	    handleEvent(event); 
	  }

	for (TLSIterator=TaskListSelectWrite.begin(); 
	     TLSIterator!=TaskListSelectWrite.end(); 
	     ++TLSIterator)
	  if (FD_ISSET(TLSIterator->first, &writefds)) {
	    Event event(TLSIterator->second.eventID, 
			timeTics, 
			TLSIterator->second.task);
	    handleEvent(event); 
	  }

	for (TLSIterator=TaskListSelectError.begin(); 
	     TLSIterator!=TaskListSelectError.end(); 
	     ++TLSIterator)
	  if (FD_ISSET(TLSIterator->first, &errorfds)) {
	    Event event(TLSIterator->second.eventID, 
			timeTics, 
			TLSIterator->second.task);
	    handleEvent(event); 
	  }

	// look at the next timeout
	setNextEvent();
      }
      else 
	if (retval == -1)
	  perror("scheduler: select failed");
    }
  }
}


/*
 * Event to trigger actions
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

#include "event.h"

Event::Event()
{
  EventPtr = new _Event();
  ref_counter = new (uint32);
  (*ref_counter) = 1;
}

Event::Event(const Event& _E)
{
  EventPtr = _E.EventPtr;
  ref_counter = _E.ref_counter;
  (*ref_counter)++;
}

Event::Event(const uint32 ID, const uint32 to, Task* task, bool active)
{
  EventPtr = new _Event(to, task, ID, active);
  ref_counter = new (uint32);
  (*ref_counter) = 1;
}

Event::Event(_Event* ePtr)
{
  EventPtr = ePtr;
  ref_counter = new (uint32);
  (*ref_counter) = 1; // hope this is the first one
}

Event::~Event()
{
  (*ref_counter)--;
  if (!(*ref_counter)) {
    delete EventPtr;
    delete ref_counter;
  }
}

Event Event::clone()
{
  Event cloneEvent(EventPtr->ID,EventPtr->timeout, EventPtr->task, EventPtr->active);
  return cloneEvent;
}

_Event* Event::ptr()
{
  return (EventPtr);
}

void Event::setTask(Task* t)
{
  EventPtr->task = t;
}

Task* Event::getTask()
{
  return (EventPtr->task);
}

void Event::setTimeout(uint32 to)
{
  EventPtr->timeout = to;
}

uint32 Event::getTimeout() const
{
  return(EventPtr->timeout);
}

void Event::setInactive()
{
  EventPtr->active = false;
}

bool Event::isActive() const
{
  return(EventPtr->active);
}

Event& Event::operator=(const Event& _E)
{
  if (this != &_E) {
    (*ref_counter)--;
    if (!(*ref_counter)) {
      delete EventPtr;
      delete ref_counter;
    }
    
    EventPtr = _E.EventPtr;
    ref_counter = _E.ref_counter;
    (*ref_counter)++;
  }

  return (*this);
}

uint32 Event::getID() const
{
  return (EventPtr->ID);
}

void Event::setID(uint32 ID)
{
  EventPtr->ID = ID;
}


bool Event::isExpired(uint32 tics) const
{
  return (EventPtr->timeout <= tics);
}

bool Event::operator==(const Event& _E) const
{
  return (EventPtr == _E.EventPtr);
}

bool isEventEqual::operator()(Event a, Event b) const
{
  if (a.getTimeout() == b.getTimeout())
    return (true);
  return (false);
}

bool isEventLess::operator()(Event a, Event b) const
{
  if (a.getTimeout() > b.getTimeout())
    return (true);
  return (false);
}

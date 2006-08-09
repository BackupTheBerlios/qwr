/*
 * event to trigger actions
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

#ifndef Event_h
#define Event_h

#include "definitions.h"

class Task;

//! internal Event Class
/*! will be represented as Class Event to the outside */
class _Event {

 public:
  uint32      timeout;
  bool        active;
  Task*       task;
  uint32      ID;

  _Event(uint32 _timeout=0, Task* _task=0, uint32 _ID=0, bool _active = true)
    : timeout(_timeout), active(_active), task(_task), ID(_ID) {}

};

//! Event Class
class Event {
 private:
  uint32*  ref_counter;
  _Event*  EventPtr;

 public:
  enum stdID {
    nil     = 0,
    timeout = 1,
    data    = 2, // + 0..17
    signal  = 18 // + 0..17
  };

  Event();
  Event(const Event& _E);
  Event(const uint32 ID, const uint32 to=0, Task* task=0, bool active=true);
  Event(_Event* ePtr);

  virtual ~Event();

  Event clone();

  void setTask(Task* t);
  void setTimeout(uint32 to);
  void setInactive();

  Task* getTask();

  bool isActive() const;
  bool isExpired(uint32 time) const;
  uint32 getTimeout() const;

  void setID(uint32 ID);
  uint32 getID() const;

  //! returns the corresponding Event
  /*! only for internal usage */
  _Event* ptr();

  // operators
  Event& operator=(const Event& _E);
  bool operator==(const Event& _E) const;

};

struct isEventEqual
{
  bool operator()(Event a, Event b) const;
};

struct isEventLess
{
  bool operator()(Event a, Event b) const;
};


#endif

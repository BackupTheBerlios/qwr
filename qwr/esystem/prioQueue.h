/*
 * derived from std priority queue for task disabling
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

#ifndef prioQueue_h
#define prioQueue_h

#include <queue>
#include <vector>

#include "event.h"
#include "task.h"

class prioQueue : public std::priority_queue<Event, std::vector<Event>, isEventLess> {

 public:
  void disableEvents(Task* task);
  void disableEvents(Task* task, unsigned int eventID);

};


inline void prioQueue::disableEvents(Task* task)
{
  std::vector<Event>::iterator it;
  for(it=c.begin(); it!=c.end(); ++it)
    if (it->getTask() == task)
      it->setInactive();
}

inline void prioQueue::disableEvents(Task* task, unsigned int  eventID)
{
  std::vector<Event>::iterator it;
  for(it=c.begin(); it!=c.end(); ++it)
    if ((it->getTask() == task) && (it->getID() == eventID))
      it->setInactive();
}

#endif

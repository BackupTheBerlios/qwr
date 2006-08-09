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

#include "task.h"
#include "scheduler.h"

Task::Task(Scheduler* _scheduler)
  : schedulerTime(_scheduler->getTimePtr()), scheduler(_scheduler)
{
}

Task::~Task()
{
}

void Task::setEvent(Event event)
{
  if (!event.getTask())
    event.setTask(this);

  scheduler->addEvent(event);
}

std::string& Task::getTaskName()
{
  return(name);
}

void Task::setTaskName(const std::string& _name)
{
  name = _name;
}

uint32 Task::getTics()
{
  return(*schedulerTime);
}

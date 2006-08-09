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

#include "simpleConnection.h"

SimpleConnection::SimpleConnection(Scheduler* _scheduler)
{
  taskInfo[0].scheduler = _scheduler;
  taskInfo[1].scheduler = _scheduler;
}

SimpleConnection::~SimpleConnection()
{
}

void SimpleConnection::inform(unsigned char taskID) {
  Event event(taskInfo[taskID].eventID, 0, taskInfo[taskID].task);
  taskInfo[taskID].scheduler->addEvent(event);
}

uint32 SimpleConnection::receive(unsigned char taskID)
{
  return(0);
}

void SimpleConnection::lockTaskQueue(unsigned char taskID)
{
}

void SimpleConnection::unlockTaskQueue(unsigned char taskID)
{
}

void SimpleConnection::_addTask(unsigned char taskID)
{
}

void SimpleConnection::_delTask(unsigned char taskID)
{
}

/*
 * connection between a itask and and a normal task 
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

#include "threadConnection.h"
#include "scheduler.h"

ThreadConnection::ThreadConnection(Scheduler* _scheduler)
  : scheduler(_scheduler)
{
  pipeInfo[0].desc[0]=-1;
  pipeInfo[0].desc[1]=-1;
  pipeInfo[1].desc[0]=-1;
  pipeInfo[1].desc[1]=-1;
  taskInfo[0].scheduler = scheduler;
  taskInfo[1].scheduler = scheduler;

}

ThreadConnection::~ThreadConnection()
{
}

void ThreadConnection::inform(unsigned char taskID) 
{
  write(pipeInfo[taskID].desc[1], &taskInfo[taskID].eventID, sizeof(uint32));
}

uint32 ThreadConnection::receive(unsigned char taskID) 
{
  uint32 eventID;
  read(pipeInfo[taskID].desc[0], &eventID, sizeof(uint32));
  return(eventID);
}

void ThreadConnection::lockTaskQueue(unsigned char taskID)
{
  Lock[taskID].enter();
}

void ThreadConnection::unlockTaskQueue(unsigned char taskID)
{
  Lock[taskID].leave();
}

void ThreadConnection::_addTask(unsigned char taskID)
{
  if (pipeInfo[taskID].desc[0] == -1) {

    // open the Pipe
    if ( pipe(pipeInfo[taskID].desc) <0) {
      perror("open pipe: ");
      return;
    }
    
    taskInfo[taskID].scheduler->
      connectDescriptor(taskInfo[taskID].task, 
			pipeInfo[taskID].desc[0], 
			taskInfo[taskID].eventID);

  }
}

void ThreadConnection::_delTask(unsigned char taskID)
{
  if (pipeInfo[taskID].desc[0] == -1) {
    scheduler->disconnectDescriptor(pipeInfo[taskID].desc[0]);
    close(pipeInfo[taskID].desc[0]);
    close(pipeInfo[taskID].desc[1]);
    pipeInfo[taskID].desc[0] = -1;
    pipeInfo[taskID].desc[1] = -1;
  }
}


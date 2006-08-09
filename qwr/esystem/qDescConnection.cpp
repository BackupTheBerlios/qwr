/*
 * connection between a qt-thread and a task
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

#include "qDescConnection.h"
#include "scheduler.h"

QDescConnection::QDescConnection(Scheduler* _scheduler)
{
  taskInfo[0].scheduler = _scheduler;
  pipeInfo[0].desc[0]=-1;
  pipeInfo[0].desc[1]=-1;
  pipeInfo[1].desc[0]=-1;
  pipeInfo[1].desc[1]=-1;
}

QDescConnection::~QDescConnection()
{
}

void QDescConnection::inform(unsigned char taskID) 
{
  write(pipeInfo[taskID].desc[1], &taskInfo[taskID].eventID, sizeof(uint32));
}

uint32 QDescConnection::receive(unsigned char taskID) 
{
  uint32 eventID;
  read(pipeInfo[taskID].desc[0], &eventID, sizeof(uint32));
  return(eventID);
}

void QDescConnection::lockTaskQueue(unsigned char taskID)
{
  Lock[taskID].lock();
}

void QDescConnection::unlockTaskQueue(unsigned char taskID)
{
  Lock[taskID].unlock();
}

void QDescConnection::_addTask(unsigned char taskID)
{
  if (taskID >0) {
    std::cerr<<"QDescConnection::_addTask WARNING can't connect a second task";
    return;
  }

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

void QDescConnection::_delTask(unsigned char taskID)
{
  if (taskID >0) {
    std::cerr<<"QDescConnection::_delTask WARNING can't delete task with ID "<<taskID<<std::endl;
    return;
  }

  if (pipeInfo[taskID].desc[0] == -1) {
    taskInfo[taskID].scheduler->
      disconnectDescriptor(pipeInfo[taskID].desc[0]);
    close(pipeInfo[taskID].desc[0]);
    close(pipeInfo[taskID].desc[1]);
    pipeInfo[taskID].desc[0] = -1;
    pipeInfo[taskID].desc[1] = -1;
  }
}

int QDescConnection::openExternalConnection()
{
  unsigned char taskID=1;
  if (pipeInfo[taskID].desc[0] == -1) {

    // open the Pipe
    if ( pipe(pipeInfo[taskID].desc) <0) {
      perror("open pipe: ");
      return(-1);
    }

    // receiver pipe for the external module
    return(pipeInfo[taskID].desc[0]);
  }
  else
    std::cerr << "QDescConnection::openExternalConnection: WARNING connection is open yet\n";
  return (-1);

}

void QDescConnection::sendMessage(Message* message)
{
  char taskID = 0;
  lockTaskQueue(taskID);
  taskInfo[(unsigned char)taskID].messageQueue.push(message);
  unlockTaskQueue(taskID);
  inform(taskID);
  return;
}

void QDescConnection::receiveMessage(Message* &message)
{
  char taskID=1;
  receive(taskID);
  lockTaskQueue(taskID);
  message = taskInfo[(unsigned char)taskID].messageQueue.front();
  taskInfo[(unsigned char)taskID].messageQueue.pop();
  unlockTaskQueue(taskID);
  return;    
}

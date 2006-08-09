/*
 * basic connection between two tasks
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

#include "connection.h"
#include "scheduler.h"

Connection::Connection(Scheduler* scheduler1, Scheduler* scheduler2)
{
  if (scheduler1) {
    taskInfo[0].scheduler = scheduler1;
    if (scheduler2)
      taskInfo[1].scheduler = scheduler2;
    else
      taskInfo[1].scheduler = scheduler1;
  }
}

unsigned char Connection::getTaskID(Task* task)
{
  if (taskInfo[0].task == task)
    return(0);

  if (taskInfo[1].task == task)
    return(1);

  return(2);
}

unsigned char Connection::getOtherTaskID(Task* task)
{
  if (taskInfo[1].task == task)
    return(0);

  if (taskInfo[0].task == task)
    return(1);

  return(2);
}

Connection::~Connection()
{
  if (taskInfo[0].task != 0)
    delTask(taskInfo[0].task);

  if (taskInfo[0].task != 0)
    delTask(taskInfo[0].task);
}

void Connection::delTask(Task* task)
{
  int taskID;
  if ((taskID = getTaskID(task)) != -1) {

    // noone will ever receive this data;
    while (!taskInfo[taskID].messageQueue.empty()){
      delete (taskInfo[taskID].messageQueue.front());
      taskInfo[taskID].messageQueue.pop();
    }
    // advice scheduler to delete the events for the messages:
    taskInfo[taskID].scheduler->delEvents(taskInfo[taskID].task, taskInfo[taskID].eventID);

    taskInfo[taskID].task = 0;
  }
}

void Connection::addTask(Task* _task, uint32 _eventID, Scheduler* scheduler)
{
  unsigned char act;
  if (!taskInfo[0].task)
    act=0;
  else
    if (!taskInfo[1].task)
      act=1;
    else {
      std::cerr<<"Connection::addTask: all tasks are in use can't add any more\n";
      return;
    }

  taskInfo[act].task = _task;
  taskInfo[act].eventID = _eventID;
  if (scheduler) {
    if (taskInfo[act].scheduler) 
      std::cerr<<"Connection::addTask: WARNING scheduler will be replaced\n";
    taskInfo[act].scheduler = scheduler;
  }
  _addTask(act);
  return;
}

void Connection::sendMessage(Message* message, Task* task) // senderTask
{
  char taskID;
  if ((taskID = getOtherTaskID(task)) < 2) {
    lockTaskQueue(taskID);
    taskInfo[(unsigned char)taskID].messageQueue.push(message);
    unlockTaskQueue(taskID);
    inform(taskID);
    return;
  }

  std::cerr<<"Connection::sendMessage WARNING task not found\n";
}

void Connection::receiveMessage(Message* &message, Task* task) // receiverTask
{
  unsigned char taskID;
  if ((taskID = getTaskID(task)) < 2) {
    receive(taskID);
    lockTaskQueue(taskID);
    message = taskInfo[taskID].messageQueue.front();
    taskInfo[taskID].messageQueue.pop();
    unlockTaskQueue(taskID);
    return;
  }    

  std::cerr<<"Connection::receiveMessage WARNING task not found\n";
}


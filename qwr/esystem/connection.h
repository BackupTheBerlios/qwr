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

#ifndef connection_h
#define connection_h

#include <queue>

#include "message.h"
#include "task.h"

class Connection {

 protected:
  Scheduler* scheduler[2];

  class TaskData {
    
  public:
    Task*      task;      // connected task
    Scheduler* scheduler; // scheduler
    uint32     eventID;   // event ID, which is set on receiver event
    std::queue<Message*> messageQueue; // messages for task
  
    TaskData()
      : task(0), scheduler(0), eventID(0){}

      TaskData(Task* _task, uint32 _eventID, Scheduler* _scheduler)
	: task(_task), scheduler(_scheduler), eventID(_eventID) {}
  
      ~TaskData()
	{
	  while (!messageQueue.empty()){
	    delete (messageQueue.front());
	    messageQueue.pop();
	  }
	}
  };

  TaskData taskInfo[2];

  unsigned char getTaskID(Task* task);
  unsigned char getOtherTaskID(Task* task);

  virtual void   inform(unsigned char taskID) = 0;
  virtual uint32 receive(unsigned char taskID) = 0;

  virtual void lockTaskQueue(unsigned char taskID) = 0; 
  virtual void unlockTaskQueue(unsigned char taskID) = 0;

  virtual void _addTask(unsigned char taskID) = 0;
  virtual void _delTask(unsigned char taskID) = 0;

 public:
  Connection(Scheduler* scheduler1=0, Scheduler* scheduler2=0);
  virtual ~Connection();

  void addTask(Task* _task, uint32 _eventID = 0, Scheduler* scheduler=0);
  void delTask(Task* _task);

  void sendMessage(Message* message, Task* senderTask);
  void receiveMessage(Message* &message, Task* receiverTask);

};

#endif

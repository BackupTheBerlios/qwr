/*
 * abstract video layer class
 * Copyright (C) 2005-2006 Sulejman Mundzic 
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 */

#ifndef abstractVideoLayer_h
#define abstractVideoLayer_h

#include "sysdefs.h"
#include "task.h"

class abstractVideoLayer : public Task {

 private:
    static    bool  initialized;

 protected:
    unsigned int    dpyFormat;
    std::string     devName;


    virtual char startCapture() = 0;
    virtual char stopCapture() = 0;

    void setDeviceName(std::string& name);

    virtual void initSchedule() = 0;

    //! timer will be used to close the soundcard
    /* if a packet arrives and is played at the soundcard,
       it will not be closed automatically, therefor a timeout
       will close the soundcard after a short while. 
       Recording will be be stopped by a user message. */
    virtual void timerHandler(Event e) = 0;

    //! Handler to hand out a sample to the soundcard
    /*! this handler receives a packet from an upper layer
      to be written to the soundcard. */
    virtual void upperQueueHandler(Event e) = 0;

    virtual void initControlPlane();

    virtual void signal3Handler(Event e)=0;

    //! Event handler for video
    /*! This handler will handle the following events:
      -# handles start/stop messages for video layer 
      -# handles packets from capture device
      -# handles timer events
    */
    virtual void EventHandler(Event event);

 public:
    abstractVideoLayer(abstractScheduler* sched = 0);
    virtual ~abstractVideoLayer();

    virtual Layer* clone() = 0;

    virtual void controlPlaneMsgHandler(controlMsg& msg) = 0;

};

inline void abstractVideoLayer::setDeviceName(std::string& name)
{ devName = name;}


#endif

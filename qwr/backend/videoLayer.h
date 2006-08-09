/*
 * abstract Video Layer Class
 * Copyright (C) 2005-2006 Sulejman Mundzic, Joern Seger 
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

#ifndef VIDEO_LAYER_H
#define VIDEO_LAYER_H

#include <string>
#include <vector>
#include <deque>

#include"pixmapFormat.h"

#include"videoCapture.h"
#include"v4lCapture.h"
#include"v4l2Capture.h"

#include"avCodecHandler.h"

#include"FactoryTPL.h"
#include"abstractVideoLayer.h"

class VideoLayer : public abstractVideoLayer {

 public:
    struct DpyMsg 
    {
	DpyMsg() : msgId(0){};
	DpyMsg(PixmapFormat::PixelFormat id)
	    : msgId(0), fmt_id(id) {};
	void set_id(int id){msgId = id;};
	int                             msgId;
	PixmapFormat::PixelFormat      fmt_id;
    };

    struct ResizeMsg 
    {
	ResizeMsg() : msgId(1),width(0),height(0)  {};
	ResizeMsg(int width, int height)
	    :msgId(1), width(width),height(height) {};
	void set_id(int id){msgId = id;};
	int                             msgId;
	int                             width;
	int                            height;
    };
/* **************** outgoing  messages ********************** */

    struct PictureFrame
    {
	PictureFrame() : seqNo(0), picture(0) {};
	PictureFrame(VideoFrame* frame)
	    : seqNo(0), picture(frame) {seqNo = seqNoCounter++;};
	static void resetSeq(){seqNoCounter = 0;};
	static int             seqNoCounter;
	int                           seqNo;
	VideoFrame*                 picture;
    };



    struct SetDpySizeMsg
    {
	SetDpySizeMsg() : msgId(51),dpy_fmtid(0),width(0),height(0)  {};
	SetDpySizeMsg(unsigned int fmtid, int width, int height)
	    :msgId(51), dpy_fmtid(fmtid),width(width),height(height) {};
	void set_id(int id){msgId = id;};
	int                             msgId;
	unsigned int                dpy_fmtid;    
	int                             width;
	int                            height;
    };



 private:

 protected:
    /* video capture */
    int                         capture_desc;
    VideoCapture*               capture_dev;
    PixmapFormat                capture_format;


    /* ximage data structures */
    PixmapFormat::PixelFormat   dpy_fmt_id;

    /* display window sizes */
    int                         winWidth;
    int                         winHeight;

    std::vector<void *>         messageBroker;
    std::deque<void *>          frameQueue;
    std::deque<PictureFrame *>  pictureQueue;
    int                         frameSeqNo;


    /* codec handling class */
    AVCodecHandler              *coder;
    int                         packet_counter;


    bool setCaptureFormat(/* int w, int h, int fmtid */);


    virtual char startCapture();
    virtual char stopCapture();
    
    virtual void initSchedule();


    virtual void timerHandler(Event e){};

    virtual void descHandler(Event e);
    virtual void upperQueueHandler(Event e);

    virtual void signal3Handler(Event e){};

    bool putFrame(VideoFrame* frame);


 public:
    VideoLayer(abstractScheduler* sched = 0);
    ~VideoLayer();
    
    virtual Layer* clone()
	{return (new VideoLayer(sched)); }
    
    virtual void controlPlaneMsgHandler(controlMsg& msg);

    void         decodeFrame(unsigned char* data_ptr, unsigned int length);


    void         postMessage(int msgId, void* msg);
    void*        getFrameItem();
};

#define init_VideoLayer layerFactoryObject.addPrototype("VideoLayer",new VideoLayer());

#endif // VIDEO_LAYER_H

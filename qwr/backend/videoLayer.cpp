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

#include "videoLayer.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/fcntl.h>

#include <string>
#include <protosim/defaultAddress.h>

#include"libs/slog.h"
#include"datapacket.h"
#include"videoControlMsg.h"


int VideoLayer::PictureFrame::seqNoCounter=0;

VideoLayer::VideoLayer(abstractScheduler* _sched)
    : abstractVideoLayer(_sched),
      capture_desc(-1),
      capture_dev(NULL),
      coder(NULL),
      capture_format(PixmapFormat::PIXFMT_YUV420P, 320, 240),
//    capture_format(PixmapFormat::PIXFMT_BGR32, 320, 240),
      dpy_fmt_id(PixmapFormat::PIXFMT_NONE),
      packet_counter(0)
{
    name = "VideoLayer";
    slog(Slog::levelDebug)<<"VideoLayer: " << "initalizing video system\n";
}

VideoLayer::~VideoLayer()
{
    capture_dev->close();
    delete coder;
}

void VideoLayer::initSchedule()
{
    sendrecvPipeLayer::initSchedule();
    messageBroker.push_back(0);
    messageBroker.push_back(0);
    messageBroker.push_back(0);
    messageBroker.push_back(0);

//  capture_dev = new v4lCapture();
//  capture_dev = new v4l2Capture();

//  capture_dev->open("/dev/video0"); // <- not here due to simulation
}




/*****************************************************************************/

void  VideoLayer::postMessage(int msgId, void* msg)
{

    slog(Slog::levelDebug)<<"VideoLayer::postMessage() \n ";

    slog(Slog::levelDebug) <<"messageBroker.size() "
			   << messageBroker.size() << "\n";
    messageBroker[msgId] = msg;

    if (write(r_pipeSocketExtern, &msgId, sizeof(int)) < 0)
	 perror("qtVideoWidget::write receive failed ");
}

void*  VideoLayer::getFrameItem()
{
    slog(Slog::levelDebug)<<"VideoLayer::getFrameItem() \n ";

    slog(Slog::levelDebug) <<"frameQueue.size() "
			   << frameQueue.size() << "\n";

    void* frameItem = frameQueue.front();
    frameQueue.pop_front();
    return frameItem;
}


bool VideoLayer::setCaptureFormat(/* unsigned int w, int h, int fmtid */)
{
    slog(Slog::levelDebug)<<"VideoLayer::setCaptureFormat()\n";
//  capture_format.setFormat(PixmapFormat::PIXFMT_YUV420P, 320, 240);
    capture_format.print();
}



bool VideoLayer::putFrame(VideoFrame* frame)
{
    slog(Slog::levelDebug)<<"VideoLayer::putFrame()\n";

    frameQueue.push_back(new PictureFrame(frame));

    char v = 'v';
    if (write(s_pipeSocketIntern, &v, 1) < 0)
	perror("VideoLayer: write receive failed ");
}

void VideoLayer::decodeFrame(unsigned char* data_ptr, unsigned int size)
{
    slog(Slog::levelDebug) << "VideoLayer::decodeFrame() \n";

    VideoFrame* display_frame = coder->decode_video_frame(data_ptr, size);
    if (display_frame == NULL)
    {
	fprintf(stderr,"display_frame = %p returning \n", display_frame);
	return;
    }
    putFrame(display_frame);
}

void VideoLayer::upperQueueHandler(Event e)
{
    slog(Slog::levelDebug) << "VideoLayer::upperQueueHandler()\n";

    while(!UpperQueue_empty())
    {

	slog(Slog::levelDebug)<<"-";
	transferItem tr(getFromUpper());
	tr.dec_layer();
 
	datapacket* pack = GETIPACKTYPE(datapacket*,tr);

	if (pack == 0)
	    continue;

	char*        frame_buffer_ptr = pack->getData();
	unsigned int length           = pack->getTailLength();

	decodeFrame((unsigned char*)frame_buffer_ptr, length);
    }
}


void VideoLayer::descHandler(Event e)
{
    slog(Slog::levelDebug)<<"VideoLayer::descHandler()\n";

    if (e.getID() == r_pipeSocketIntern)
    {

	int id;
	DpyMsg*     dpyMsg    = NULL;
	ResizeMsg*  resizeMsg = NULL;

	read(r_pipeSocketIntern, &id, sizeof(int));
	switch(id)
	{
	    case 0:
		dpyMsg = (DpyMsg*)messageBroker[0];
		dpy_fmt_id = dpyMsg->fmt_id;
		fprintf(stderr,"dpy_format.id() = %d \n", dpy_fmt_id);
		coder    = new AVCodecHandler();
		coder->init_decoder(dpy_fmt_id);
		delete dpyMsg;
		break;
	    case 1:
		resizeMsg = (ResizeMsg*)messageBroker[1];
		winWidth  = resizeMsg->width;
		winHeight = resizeMsg->height;
		delete resizeMsg;
		break;
	    default:
		return;

	}
	return;
    }


    if (capture_desc == 0)
    {
	fprintf(stderr,"video capture invalid state - returning \n");
	return;
    }

    slog(Slog::levelDebug)<<name<<": gathering captured "<<" frames\n";

    struct VideoFrame* captured_frame = capture_dev->nextframe();

    slog(Slog::levelDebug)<<"frame captured \n";

    if (captured_frame == NULL)
    {
	fprintf(stderr,"video capture failed - returning \n");
	return;
    }

    int fps = (captured_frame->seqNo + 1) * 1000 / captured_frame->msec_ts;
    fprintf(stderr,"fps = %d \n",fps);


    VideoFrame*  preview_frame = coder->convert_video_frame(captured_frame);
    putFrame(preview_frame);

    FrameSlice* slice = coder->encode_video_frame(captured_frame);
    fprintf(stderr,"slice->length() = %d \n", slice->length());


    if (slice == NULL)
    {
	return;
    }
    fprintf(stderr,"slice->length() = %d \n", slice->length());

/*
    packet_counter++;
    int loss_packet = packet_counter % 21;
    if (!loss_packet) return;
*/

    int              rest_size = slice->length();
    const char*      chunk_ptr = slice->data();
    int              quantum   = 2048;
    unsigned short   copy_size;
    char*            chunk;


    while( rest_size > 0 )
    {
	copy_size = rest_size/quantum ? quantum : rest_size;
	chunk = new char[quantum + 2];
	if( chunk == NULL ) break;
	memcpy(chunk + 2, chunk_ptr,copy_size);
	*((unsigned short*)chunk) = (unsigned short)copy_size+2;
	chunk_ptr = chunk_ptr + copy_size;
	rest_size = rest_size - copy_size;
#if 0
	packet_counter++;
	int loss_packet = packet_counter % 15;
	if (!loss_packet) continue;
#endif
	transferItem tr;
	tr.setdata(new ICI(0,0,(unsigned short)copy_size,2,false));
	tr.setRaw((char *)chunk,copy_size+2);
	
	std::vector<advLayer*> to;
	bool ambigAddress;
	defaultAddress dAddr;
	
	if (UpperLayerBox->findLayers(dAddr,to,ambigAddress))
	    to[0]->transferFromLower(tr,0);

    }
    delete slice;
    return;
}



char VideoLayer::startCapture()
{
    slog(Slog::levelDebug) << "VideoLayer::startCapture()\n";

    setCaptureFormat();

    /* open capture device */
    /*
    if (capture_dev == NULL)
    {
	capture_dev = new v4l2Capture();
	if (capture_dev->open("/dev/video0") == -1)
	{
	    delete capture_dev;
	    capture_dev = new v4lCapture();
	    if (capture_dev->open("/dev/video0") == -1)
	    {
		fprintf(stderr,"can not handeln capture \n");
		delete capture_dev; capture_dev = NULL;
		return false;
	    }
	}
    }
    */
    capture_dev = new v4lCapture();
//  capture_dev = new v4l2Capture();
    capture_dev->open("/dev/video0");


    if( capture_dev->setformat(capture_format) != 0)
    {
	fprintf(stderr,"capture format not supported \n");
	capture_format.print();
	return false;
    }
    capture_format.print();
    coder->init_encoder(capture_format);
    if ((capture_desc = capture_dev->startvideo(-1,2)) == -1)
    {
	return false;
    }
    fprintf(stderr,"capture_desc = %d \n", capture_desc);
    sched->add_select(this, capture_desc);
    return true;
}

char VideoLayer::stopCapture()
{
    slog(Slog::levelDebug)<<name<<": closing capture device\n";

    capture_dev->stopvideo();
    capture_dev->close();
    sched->del_select(capture_desc);
    capture_desc = 0;

//    coder->fini_encoder();
    return (0);
}


void VideoLayer::controlPlaneMsgHandler(controlMsg& msg)
{
    slog(Slog::levelDebug)<<"VideoLayer::controlPlaneMsgHandler\n";
    /**** start/stop capture *******/

    if (msg.getName() == "Capture") {
	CaptureMsg* vm = dynamic_cast<CaptureMsg*>(&msg);
	if (vm && (vm->status == CaptureMsg::start))
	{
	    startCapture();
	}
	else if (vm && (vm->status == CaptureMsg::stop))
	{
	    stopCapture();
	}
	else slog(Slog::levelWarning)<<name<<": wrong control message type;\n";
    }

    return;
}

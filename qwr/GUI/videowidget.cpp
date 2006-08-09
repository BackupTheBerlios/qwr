/*
 * video widget class 
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

#include <qpainter.h>
#include <stdio.h>
#include <assert.h>

#include "videowidget.h"
#include "qDescConnection.h"
#include "videoDisplayMsg.h"


videoWidget::videoWidget( QWidget *parent, const char *name )
    : QWidget( parent, name ),
      _shm(NULL), 
      _ratio_x(4), _ratio_y(3),
      _no_mitshm(false), 
      _ximage(0),
      _imgPixelSize(0)
{
    fprintf(stderr, "videoWidget:videoWidget()\n");

    _dpy    = x11Display();
    _win    = winId();
    _gc     = XCreateGC(x11Display(), winId(), 0, NULL);
    _vinfo  = (XVisualInfo*)malloc(sizeof(XVisualInfo));
    visualPropertiesIdentify(); // sets up _vinfo
    //    findPixmapFormat();         // find PIX_FMT_ID


    if (!XShmQueryExtension(x11Display()))
    {
	_no_mitshm = true;
	fprintf(stderr,"videoWidget: no_mitshm=1 \n");
    }

    setPalette( QPalette( QColor( 100, 100, 100) ) );

    fprintf(stderr,"dpy = %p \n", x11Display());
    fprintf(stderr,"win = %d \n", winId());

    /* find out suported image formats by the display */
    initXvImage();
}



void videoWidget::setConnector(QDescConnection* _connection)
{
    fprintf(stderr, "videoWidget::setVideoDevice()\n");
    connection = _connection;
    
    // Connect notifier
    int receivSoc = connection->openExternalConnection();
    videoNotifier = new QSocketNotifier(receivSoc,QSocketNotifier::Read);
    connect( videoNotifier, SIGNAL(activated(int)),
	     this, SLOT(videoDevMessageReader(int)));

    // send message to the video coordinator
    VideoDisplayMsg::Dpy* dpyMsg = new  VideoDisplayMsg::Dpy(_imgFormat.id());
    
    fprintf(stderr,"dpyMsg->msgId = %d, dpyMsg->dpy_fmtid = 0 \n",
	    dpyMsg->getID() /* ,dpyMsg->dpy_fmtid */);


    connection->sendMessage(dpyMsg);
}


void videoWidget::resizeEvent(QResizeEvent *)
{
    fprintf(stderr,"videoWidget::resizeEvent()\n");

    _winWidth   = width();
    _winHeight  = height();
    _wx         = 0;
    _wy         = 0;
    _ww         = _winWidth;
    _wh         = _winHeight;

    ratio_fixup(&_ww, &_wh, &_wx, &_wy);

    /* pass the current window sizes to backend parts */
//    VideoLayer::ResizeMsg* resizeMsg =
//	new VideoLayer::ResizeMsg(width(), height());

//    fprintf(stderr,"resizeMsg->msgId = %d \n", resizeMsg->msgId);

//    _videoDev->postMessage(resizeMsg->msgId, resizeMsg);
}


void videoWidget::videoDevMessageReader(int socket)
{
  //    fprintf(stderr, "videoWidget::videoDevMessageReader()\n");
    
    /*
    char videoMsg[6];
    char msg_id;
    int  len;
    */

    Message* msg;
    connection->receiveMessage(msg);

    //fprintf(stderr, "videoWidget: videoMsg = %d \n", msg->getID());


    // what happens here?? strange!! msg id is not tested! only one case? - JS

    if (msg->getID() == VideoDisplayMsg::type_frame) {

      VideoDisplayMsg::PictureFrame* frame = static_cast<VideoDisplayMsg::PictureFrame*>(msg);

      if (_imgFormat  != frame->picture->fmt)
	{
	  _imgFormat = frame->picture->fmt;
	  if (_ximage != NULL)
	    destroyX11image();
	  resize(_imgFormat.width()+10, _imgFormat.height()+10);
	  show();
	  createX11Image(_imgFormat.width(), _imgFormat.height());
	}
      //fprintf(stderr,"videoWidget: data_ptr = %d\n", frame->picture->data_ptr); 
      memcpy(_ximage->data, frame->picture->data_ptr, _imgFormat.size());

      putFrame();
      
      free(frame->picture->data_ptr);
      delete frame->picture;
      delete frame;
    }

    return;
}

void videoWidget::paintEvent( QPaintEvent * )
{
    //fprintf(stderr, "videoWidget::paintEvent()\n");
    if (_ximage != NULL)
    {
	putFrame();
    }
    else
    {
	QString s = "video stopped";
	QPainter p( this );
	p.drawText( 144, 144, s );
    }
}


QSizePolicy videoWidget::sizePolicy() const
{
    return QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
}


static int
no_mitshm_exception(Display * dpy, XErrorEvent * event)
{
    fprintf(stderr,"videoWidget:catch_no_mitshm() \n");
//  _no_mitshm++;
    return 0;
}


void videoWidget::destroyX11image()
{
    fprintf(stderr,"videoWidget::destroyX11image()\n");
    if (_shm && !_no_mitshm)
    {
	XShmDetach(_dpy, _shm);
	XDestroyImage(_ximage);
	shmdt(_shm->shmaddr);
	free(_shm);
    } else
	XDestroyImage(_ximage);
    _ximage = NULL;
}

XImage* videoWidget::createX11Image(int width, int height)
{
    fprintf(stderr,"videoWidget::createX11Image() \n");

    fprintf(stderr,"videoWidget: \n"
	    "dpy     = %p \n"
	    "vinfo   = %p \n"
	    "width   = %d \n"
	    "height  = %d \n"
	    "shm     = %p \n"
	    "ximage  = %p \n",
	    _dpy, _vinfo, width, height, _shm, _ximage);

    XImage          *ximage = NULL;
//    unsigned char   *ximage_data;
    XShmSegmentInfo *shminfo = NULL;
    int             (*old_handler)(Display* , XErrorEvent* );
    
    if (_no_mitshm)
	goto no_mitshm;

    assert(width > 0 && height > 0);
    
    old_handler = XSetErrorHandler(no_mitshm_exception);
    shminfo = (XShmSegmentInfo*)malloc(sizeof(XShmSegmentInfo));
    memset(shminfo, 0, sizeof(XShmSegmentInfo));
    
    ximage = XShmCreateImage(_dpy,_vinfo->visual,_vinfo->depth,
			     ZPixmap, NULL,
			     shminfo, width, height);


    fprintf(stderr,"videoWidget: ximage = %p \n", ximage);

    if (NULL == ximage)
	goto shm_error;
    shminfo->shmid = shmget(IPC_PRIVATE,
			    ximage->bytes_per_line * ximage->height,
			    IPC_CREAT | 0777);
    if (-1 == shminfo->shmid) {
	perror("shmget [x11]");
	goto shm_error;
    }
    shminfo->shmaddr = (char *) shmat(shminfo->shmid, 0, 0);
    if ((void *)-1 == shminfo->shmaddr) {
	perror("shmat");
	goto shm_error;
    }
    ximage->data = shminfo->shmaddr;
    shminfo->readOnly = False;
    
    XShmAttach(_dpy, shminfo);
    XSync(_dpy, False);
    if (_no_mitshm)
	goto shm_error;
    shmctl(shminfo->shmid, IPC_RMID, 0);
    XSetErrorHandler(old_handler);

    // set up image fields
    _shm    = shminfo;
    _ximage = ximage;
    fprintf(stderr,"videoWidget: \n"
	    "dpy     = %p \n"
	    "vinfo   = %p \n"
	    "width   = %d \n"
	    "height  = %d \n"
	    "shm     = %p \n"
	    "ximage  = %p \n",
	    _dpy, _vinfo, width, height, _shm, _ximage);
    return ximage;

 shm_error:
    if (ximage) {
	XDestroyImage(ximage);
	ximage = NULL;
    }
    if ((void *)-1 != shminfo->shmaddr  &&  NULL != shminfo->shmaddr)
	shmdt(shminfo->shmaddr);
    free(shminfo);
    XSetErrorHandler(old_handler);
    _no_mitshm = 1;

 no_mitshm:
    _shm = NULL;
    fprintf(stderr,"videoWidget: no_mitshm\n");
    
    char* ximage_data = (char*)malloc(_imgFormat.width() * _imgFormat.height() * _imgPixelSize);
    if(ximage_data == NULL)
    {
	fprintf(stderr,"out of memory\n");
	exit(1);
    }

    ximage = XCreateImage(_dpy, _vinfo->visual, _vinfo->depth,
			  ZPixmap, 0, ximage_data,
			  width, height,
			  8, 0);
    memset(ximage->data, 0, ximage->bytes_per_line * ximage->height);

    return ximage;
}


void videoWidget::putFrame()
{
  //    fprintf(stderr,"videoWidget::putFrame() \n");
    if (_no_mitshm)
    {
	XPutImage(_dpy,winId(),_gc,_ximage,0,0,
		  (_winWidth  - _imgFormat.width())  >> 1,
		  (_winHeight - _imgFormat.height()) >> 1,
		  _imgFormat.width(), _imgFormat.height());
    }
    else
    {
	XShmPutImage(_dpy,winId(),_gc,_ximage,0,0,
		     (_winWidth  - _imgFormat.width())  >> 1,
		     (_winHeight - _imgFormat.height()) >> 1,
		     _imgFormat.width(), _imgFormat.height(),
		     True);
    }
}

void videoWidget::visualPropertiesIdentify()
{

    fprintf(stderr,"videoWidget::visualPropertiesIdentify() \n");
    /* Display* dpy    = x11Display(); */
    x11Display(); // ?? do we need it!? - JS
    XVisualInfo    *vinfo_list = NULL;
    int            n;

    _vinfo->screen = QPaintDevice::x11AppScreen();
    vinfo_list = XGetVisualInfo(qt_xdisplay(), VisualScreenMask, _vinfo, &n);
    
    fprintf(stderr,"videoWidget: vinfo_list = %p n = %d \n", vinfo_list,n);
    for (int i = 0; i < n; i++)
      {
	//	vinfo_list[i].depth;
	fprintf(stderr,"vinfo_list[%d].depth = %d \n", i, vinfo_list[i].depth);
      } 
    
    *_vinfo = vinfo_list[0];
    XFree(vinfo_list);

    fprintf(stderr,"videoWidget: visualid=0x%lx \n", _vinfo->visualid);

    findPixmapFormat();
}

void videoWidget::findPixmapFormat()
{
    fprintf(stderr,"videoWidget::findPixmapFormat() \n");
    PixmapFormat::PixelFormat  fmtid;

    int                        n;
    XPixmapFormatValues*       pf;


    pf = XListPixmapFormats(_dpy,&n);
    for (int i = 0; i < n; i++)
    {
	if (pf[i].depth == _vinfo->depth)
	{
	    _imgPixelSize = pf[i].bits_per_pixel/8;
	    fprintf(stderr,"_imgPixelSize = %d \n", _imgPixelSize);
	}
    }

    if (ImageByteOrder(_dpy) == MSBFirst) 
    {
	    switch (_imgPixelSize)
	    {
	    case 2:
		if (_vinfo->depth==15)
		{
		    fmtid = PixmapFormat::PIXFMT_RGB15_BE;
		}
		else
		{
		    fmtid = PixmapFormat::PIXFMT_RGB16_BE;
		}
		break;
	    case 3:
		fmtid = PixmapFormat::PIXFMT_RGB24;
		break;
	    case 4:
		fmtid = PixmapFormat::PIXFMT_RGB32;
		break;
	    }
    }
    else
    {
	switch (_imgPixelSize)
	{
	    case 2:
		if (_vinfo->depth==15)
		{
		    fmtid = PixmapFormat::PIXFMT_RGB15_LE;
		}
		else
		{
		    fmtid = PixmapFormat::PIXFMT_RGB16_LE;
		}
		break;
	    case 3:
		fmtid = PixmapFormat::PIXFMT_BGR24;
		break;
	    case 4:
		fmtid = PixmapFormat::PIXFMT_BGR32;
		break;
	}
    }
    _imgFormat.setFormat(fmtid);

    fprintf(stderr,"format = %d \n", fmtid);
}




void videoWidget::initXvImage()
{
    unsigned int            im_adaptor,im_port = -1U;

    std::map<PixmapFormat::PixelFormat,unsigned int> im_formats;

    fprintf(stderr,"videoWidget::initXvImage() \n");
    unsigned int           ver, rel, req, ev, err;
    int                    formats;
    unsigned int           adaptors;
    XvImageFormatValues    *fo;
    XvAdaptorInfo          *ai;

    if (Success != XvQueryExtension(_dpy,&ver,&rel,&req,&ev,&err))
    {
	fprintf(stderr,"Xvideo: Server has no Xvideo extention support\n");
	return;
    }

    if (Success != XvQueryAdaptors(_dpy,DefaultRootWindow(_dpy),&adaptors,&ai))
    {
	fprintf(stderr,"Xvideo: XvQueryAdaptors failed");
	return;
    }
    for (unsigned int i = 0; i < adaptors; i++)
    {
	if ((ai[i].type & XvInputMask) &&
	    (ai[i].type & XvImageMask) &&
	    (im_port == -1U))
	{
	    im_port    = ai[i].base_id;
	    im_adaptor = i;
	}
    }

    if (im_port == -1U)
    {
	return;
    }

    fo = XvListImageFormats(_dpy, im_port, &formats);
    for(int i = 0; i < formats; i++)
    {
	fprintf(stderr,"xv fo.id: 0x%x (%c%c%c%c) %s",
		fo[i].id,
		(fo[i].id)       & 0xff,
		(fo[i].id >>  8) & 0xff,
		(fo[i].id >> 16) & 0xff,
		(fo[i].id >> 24) & 0xff,
		(fo[i].format == XvPacked) ? "packed" : "planar");

	if (0x32595559 == fo[i].id) {
	    im_formats[PixmapFormat::PIXFMT_YUYV] = fo[i].id;
	    fprintf(stderr," - PIXFMT_YUYV supported \n");
	}
	if (0x59565955 == fo[i].id) {
	    im_formats[PixmapFormat::PIXFMT_UYVY] = fo[i].id;
	    fprintf(stderr," - PIXFMT_UYVY supported \n");
	}
	if (0x30323449 == fo[i].id) {
	    im_formats[PixmapFormat::PIXFMT_YUV420P] = fo[i].id;
	    fprintf(stderr," - PIXFMT_YUV420P supported \n");
	}
	fprintf(stderr,"\n");
    }
}



void videoWidget::ratio_fixup(int *width, int *height, int *xoff, int *yoff)
{
    int h = *height;
    int w = *width;

    if (0 == _ratio_x || 0 == _ratio_y)
    {
	return;
    }
    if (w * _ratio_y < h * _ratio_x)
    {
	*height = *width * _ratio_y / _ratio_x;
	if (yoff)
	{
	    *yoff  += (h-*height)/2;
	}
    }
    else if (w * _ratio_y > h * _ratio_x)
    {
	*width  = *height * _ratio_x / _ratio_y;
	if (yoff)
	{
	    *xoff  += (w-*width)/2;
	}
    }
}

/*
 * video for linux interface
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

#include <iostream>

#include "v4l2Capture.h"

int v4l2Capture::xioctl(int fd, int cmd, void *arg, int mayfail)
{
    int rc;

    rc = ioctl(fd,cmd,arg);
    if (0 == rc)
	return rc;
    if (mayfail && errno == mayfail)
	return rc;
//  print_ioctl(stderr,ioctls_v4l2,PREFIX,cmd,arg);
//    fprintf(stderr,": %s\n",(rc == 0) ? "ok" : strerror(errno));
    return rc;
}


v4l2Capture::v4l2Capture(char const* device)
        : VideoCapture(device)
{
    fprintf(stderr, "v4l2Capture::v4l2Capture()\n");
    devname = device;
    fd = -1;
/*
    fd = -1;
    method = READ_IO_METHOD;
    queued   = 0;
    dequeued = 0;
*/
}


v4l2Capture::~v4l2Capture()
{
    fprintf(stderr, "v4l2Capture::~v4l2Capture()\n");
}


void v4l2Capture::get_device_capabilities()
{
  //    fprintf(stderr, "v4l2Capture::get_device_capabilities()\n");

    for (nfmts = 0; nfmts < max_format; nfmts++)
    {
	fmt[nfmts].index = nfmts;
	fmt[nfmts].type  = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if (-1 == xioctl(fd, VIDIOC_ENUM_FMT, &fmt[nfmts], EINVAL)) {
	  std::cerr<<"v4l2Capture::get_device_capabilities: ioctl failed\n";
	    break;
	}
	/*
	fprintf(stderr, "fmt[%d] = %s \n", nfmts, fmt[nfmts].description);
	fprintf(stderr, "fmt[%d] = %d \n", nfmts, fmt[nfmts].pixelformat);
	if (fmt[nfmts].pixelformat == V4L2_PIX_FMT_YVU420)
	    fprintf(stderr, "V4L2_PIX_FMT_YVU420 found \n");
	if (fmt[nfmts].pixelformat == V4L2_PIX_FMT_YUYV)
       	    fprintf(stderr, "V4L2_PIX_FMT_YUYV found \n");
	*/
    }

    streamparm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    ioctl(fd,VIDIOC_G_PARM,&streamparm);
}

void v4l2Capture::setPictureParams(int colour, int brightness,
				   int hue,    int contrast)
{
  //    fprintf(stderr, "v4l2Capture::setPictureParams() - not implemented \n");
  std::cerr << "v4l2Capture::setPictureParams() - not implemented \n";
}

/* open/close */
int  v4l2Capture::open()
{
  //    fprintf(stderr, "v4l2Capture::open()\n");

/*
    if (fd != 0)
    {
//	startvideo(-1, 2);
	return fd;
    }
*/

    if ((fd = ::open(devname, O_RDWR)) == -1)
    {
	perror("v4l2Capture::open failed");
	goto err;
    }

    if (xioctl(fd,VIDIOC_QUERYCAP,&cap,EINVAL) == -1)
    {
	fprintf(stderr,"v4l2: open %s: %s\n",devname,strerror(errno));
	goto err;
    }

    fcntl(fd,F_SETFD,FD_CLOEXEC);

    /*
    fprintf(stderr,"v4l2: device info:\n"
	    "  %s %d.%d.%d / %s @ %s\n",
	    cap.driver,
	    (cap.version >> 16) & 0xff,
	    (cap.version >>  8) & 0xff,
	    cap.version         & 0xff,
	    cap.card,cap.bus_info);
    */

    get_device_capabilities();

    return fd;

 err:
    fprintf(stderr,"drv: opening error \n");
    if (fd != -1)
	::close(fd);
    return -1;
}


/* attributes */
char const* v4l2Capture::get_devname()
{
  //    fprintf(stderr, "v4l2Capture::get_devname()\n");
    return devname; 
}

/* capture */
int v4l2Capture::setFormat(PixmapFormat& in_fmt)
{
  //    fprintf(stderr, "v4l2Capture::setformat()\n");
    
    memset (&fmt_v4l2, 0, sizeof (fmt_v4l2));

    fmt_v4l2.type                 = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt_v4l2.fmt.pix.pixelformat  = V4L2_PIX_FMT_YUYV;
    fmt_v4l2.fmt.pix.width        = in_fmt.width();
    fmt_v4l2.fmt.pix.height       = in_fmt.height();
    fmt_v4l2.fmt.pix.field        = V4L2_FIELD_ANY;
    fmt_v4l2.fmt.pix.bytesperline = 0;

    /*
    fprintf(stderr,"v4l2VideoLayer: \n"
	    "fmt_id    = %d \n"
	    "width     = %d \n"
	    "height    = %d \n"
	    "size      = %d \n",
	    
	    in_fmt.id(),
	    in_fmt.width(), in_fmt.height(),
	    in_fmt.linesize());
    */

    if (-1 == xioctl(fd, VIDIOC_S_FMT, &fmt_v4l2, EINVAL))
	return -1;

    //    fprintf(stderr,"format set \n");


    in_fmt = curr_fmt.setFormat(PixmapFormat::PIXFMT_YUV420P,
				fmt_v4l2.fmt.pix.width,
				fmt_v4l2.fmt.pix.height);
    /*
    fprintf(stderr,"v4l2VideoLayer: \n"
	    "fmt_id    = %d \n"
	    "width     = %d \n"
	    "height    = %d \n"
	    "size      = %d \n",

	    curr_fmt.id(),
	    curr_fmt.width(), curr_fmt.height(),
	    curr_fmt.linesize());

    fprintf(stderr,"v4l2: new capture params (%dx%d, %c%c%c%c, %d byte)\n",
	    curr_fmt.width(),curr_fmt.height(),
	    fmt_v4l2.fmt.pix.pixelformat & 0xff,
	    (fmt_v4l2.fmt.pix.pixelformat >>  8) & 0xff,
	    (fmt_v4l2.fmt.pix.pixelformat >> 16) & 0xff,
	    (fmt_v4l2.fmt.pix.pixelformat >> 24) & 0xff,
	    fmt_v4l2.fmt.pix.sizeimage);
    */
    return 0;
}



int v4l2Capture::queue_buffer(void)
{
  //    fprintf(stderr, "v4l2Capture::queue_buffer()\n");
    int frame = queue % reqbufs.count;
    int rc;

    if (queue - waiton != 0)
    {
	fprintf(stderr,"v4l2: waiting for a free buffer\n");
//	return -1;
    }

    rc = xioctl(fd,VIDIOC_QBUF,&buf_v4l2[frame], 0);
    if (0 == rc)
    {
	fprintf(stderr,"v4l2: buffer queued \n");
	queue++;
    }
    return rc;
}


void v4l2Capture::queue_all(void)
{
  //    fprintf(stderr, "v4l2Capture::queue_all()\n");
    for (;;)
    {
	if (queue - waiton >= reqbufs.count)
	    return;
	if (0 != queue_buffer())
	    return;
    }
}

int v4l2Capture::startVideo(int fps, unsigned int buffers)
{
    fprintf(stderr, "v4l2Capture::startvideo()\n");

    fps   = fps;
    start = 0;

    if (!(cap.capabilities & V4L2_CAP_STREAMING))
    {
	fprintf(stderr, "V4L2_CAP_STREAMING not supported \n");
	/* think about returning the fd */ 
	return -1;
    }

    //    fprintf(stderr,"v4l2Capture: start streaming \n");
    
    /* setup buffers */
    reqbufs.count  = buffers;
    reqbufs.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    reqbufs.memory = V4L2_MEMORY_MMAP;
    if (-1 == xioctl(fd, VIDIOC_REQBUFS, &reqbufs, 0))
    {
	return -1;
    }
    for (unsigned int i = 0; i < reqbufs.count; i++) {
	buf_v4l2[i].index  = i;
	buf_v4l2[i].type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buf_v4l2[i].memory = V4L2_MEMORY_MMAP;
	if (-1 == xioctl(fd, VIDIOC_QUERYBUF, &buf_v4l2[i], 0))
	{
	    return -1;
	}
	buf_me[i].fmt      = curr_fmt;
	buf_me[i].data_ptr = (unsigned char*)mmap(NULL,
				  buf_v4l2[i].length,
				  PROT_READ | PROT_WRITE, MAP_SHARED,
				  fd,
				  buf_v4l2[i].m.offset);

	if (MAP_FAILED == buf_me[i].data_ptr)
	{
	    perror("mmap");
	    return -1;
	}
	//    print_bufinfo(&buf_v4l2[i]);
    }
    /* queue up all buffers */
    queue_all();

    /* start capture */
    if (-1 == xioctl(fd,VIDIOC_STREAMON,&fmt_v4l2.type,0))
    {
	return -1;
    }

     return fd;
}



struct VideoFrame* v4l2Capture::nextFrame()
{
  //    fprintf(stderr, "v4l2Capture::nextframe()\n");

    struct VideoFrame *video_frame = NULL;
    int rc, frame = 0;

    if (cap.capabilities & V4L2_CAP_STREAMING)
    {
	queue_all();
	struct v4l2_buffer buf;

	/* get it */
	memset(&buf,0,sizeof(buf));
	buf.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buf.memory = V4L2_MEMORY_MMAP;

	if (-1 == xioctl(fd,VIDIOC_DQBUF,&buf, 0))
	{
	    return NULL;
	}
	frame = buf.index;
	waiton++;
	buf_v4l2[buf.index] = buf;
//	video_frame = &buf_me[frame];

	video_frame = new VideoFrame();
	if ( video_frame == NULL ) return NULL;
	*video_frame = buf_me[frame];
	video_frame->data_ptr = new unsigned char[buf_me[frame].fmt.size()];
	if ( video_frame->data_ptr == NULL )
	{
	    delete video_frame;
	    return NULL;
	}
	memcpy(video_frame->data_ptr, buf_me[frame].data_ptr,
	       video_frame->fmt.size());
    }
    else
    {
	video_frame = new VideoFrame();
	if (video_frame == NULL) return NULL;
	video_frame->fmt      = curr_fmt;
	video_frame->data_ptr = (unsigned char*)malloc(curr_fmt.size());
	rc = read(fd,video_frame->data_ptr,video_frame->fmt.size());
	if (rc != (int)video_frame->fmt.size())
	{
	    if (-1 == rc)
	    {
		perror("v4l2: read");
	    }
	    else
	    {
	      //  fprintf(stderr, "read: rc=%d/size=%d\n",rc,curr_fmt.size());
	    }
	    free(video_frame);
	    return NULL;
	}
    }

    return video_frame;
}


void v4l2Capture::stopVideo()
{
  //    fprintf(stderr, "v4l2Capture::stopVideo()\n");

    if (!(cap.capabilities & V4L2_CAP_STREAMING))
    {
      //	fprintf(stderr, "V4L2_CAP_STREAMING not supported \n");
      std::cerr <<"V4L2_CAP_STREAMING not supported \n";
	return;
    }

    fprintf(stderr,"v4l2Capture: stop streaming \n");

    
    /* stop capture */
    if (-1 == ioctl(fd,VIDIOC_STREAMOFF,&fmt_v4l2.type))
    {
	perror("ioctl VIDIOC_STREAMOFF");
    }
    
    /* free buffers */
    for (unsigned int i = 0; i < reqbufs.count; i++)
    {
//	print_bufinfo(&buf_v4l2[i]);
//	fprintf(stderr,"freeing the buffer i = %d \n", i);
	if (-1 == munmap(buf_me[i].data_ptr,buf_me[i].fmt.size()))
	    perror("munmap");
    }
    queue  = 0;
    waiton = 0;
}

int v4l2Capture::close()
{
    fprintf(stderr, "v4l2Capture::close()\n");

//  stopVideo();
//    fprintf(stderr, "v4l2Capture: fd = %d\n", fd);
    if (::close(fd)<0)
    {
	perror("close failed");
	return(fd = -1);
    }
    fd = -1;
    return 0;
}

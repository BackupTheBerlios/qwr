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

#include"v4lCapture.h"

std::map<PixmapFormat::PixelFormat, unsigned short> v4lCapture::v4l_pix_id;
std::map<unsigned short, PixmapFormat::PixelFormat> v4lCapture::ptt_pix_id;

int v4lCapture::xioctl(int fd, int cmd, void *arg)
{
  int rc;

  rc = ioctl(fd,cmd,arg);
  if (0 == rc)
    return 0;
  //  fprintf(stderr,"xioctl error: %s\n",(rc == 0) ? "ok" : strerror(errno));
  return rc;
}


v4lCapture::v4lCapture(char const* device)
    : VideoCapture(device)
{
  fprintf(stderr, "v4lCapture::v4lCapture()\n");
  devname = device;
  init_palette_table();
  fd = -1;
  method = READ_IO_METHOD;
  mmap = (unsigned char*)MAP_FAILED;
  queued   = 0;
  dequeued = 0;
}


v4lCapture::~v4lCapture()
{
  fprintf(stderr, "v4lCapture::~v4lCapture()\n");
  if (-1 != fd)
    if (::close(fd))
      perror("close failed");
}



void v4lCapture::get_device_capabilities()
{
  //  fprintf(stderr, "v4lCapture::get_device_capabilities()\n");

  /*
  if (capability.type & VID_TYPE_CAPTURE)
    fprintf(stderr,"size    : %dx%d => %dx%d\n",
            capability.minwidth,capability.minheight,
            capability.maxwidth,capability.maxheight);
  */
}

void v4lCapture::setPictureParams(int colour, int brightness,
                                  int hue,    int contrast)
{
  //  fprintf(stderr, "v4lCapture::setPictureParams() \n");

  xioctl(fd,VIDIOCGPICT,&pict);
  pict.colour     = colour;
  pict.brightness = brightness;
  pict.hue        = hue;
  pict.contrast   = contrast;
  xioctl(fd, VIDIOCSPICT, &pict);
}


/* open/close */
int  v4lCapture::open()
{
  fprintf(stderr, "v4lCapture::open()\n");

  if (fd > 0) return fd;

  struct stat st;

  fd = ::open(devname, O_RDWR);

  if (fd == -1)
  {
    perror("::open failed");
    goto err;
  }
  std::cerr << "open fd is "<<fd<<std::endl;

  if (fstat(fd,&st) == -1) {
    goto err;
  }

  if (!S_ISCHR(st.st_mode)) {
    goto err;
  }

  if (major(st.st_rdev) != 81) {
    goto err;
  }

  fcntl(fd,F_SETFD,FD_CLOEXEC);

  if (ioctl(fd,VIDIOCGCAP,&capability) == -1) {
    goto err;
  }
  /*
  fprintf(stderr,"size    : %dx%d => %dx%d\n",
          capability.minwidth,capability.minheight,
          capability.maxwidth,capability.maxheight);
  */

  /* picture parameters */
  setPictureParams(32768, 32256, 65535, 34816);

  map_memory_buffers();

  return fd;

 err:
  fprintf(stderr, "v4lCapture::open() - failed!!!\n");

  if (-1 != fd) {
    ::close(fd);
    fd = -1;
  }

 return -1;
}

int v4lCapture::map_memory_buffers(void)
{
    if (0 == xioctl(fd,VIDIOCGMBUF,&mbuf))
    {
      //	fprintf(stderr,"mbuf: size=%d frames=%d\n", mbuf.size, mbuf.frames);
	mmap = (unsigned char*) ::mmap(0, mbuf.size, PROT_READ|PROT_WRITE,
				       MAP_SHARED, fd, 0);
	if (mmap == MAP_FAILED) perror("mmap");
    }
    else
    {
	mmap = (unsigned char*)MAP_FAILED;
    }
    
    if (MAP_FAILED != mmap)
    {
	fprintf(stderr,"  v4l: using mapped buffers for capture\n");
	nbuf = mbuf.frames;
	buf_v4l = (struct video_mmap*)malloc(nbuf * sizeof(struct video_mmap));
	if (buf_v4l == NULL)
	{
	    return -1;
	}
	memset(buf_v4l,0,nbuf * sizeof(struct video_mmap));
	buf_me = new VideoFrame[nbuf];
	if (buf_me == NULL)
	{
	    free(buf_v4l);
	    return -1;
	}
	memset(buf_me, 0, nbuf * sizeof(struct VideoFrame));
	method = MMAP_IO_METHOD;
    }
    else
    {
	fprintf(stderr,"  v4l: using read() for capture\n");
	method = READ_IO_METHOD;
    }

    return 0;
}


/* attributes */
char const* v4lCapture::get_devname()
{
  fprintf(stderr, "v4lCapture::get_devname()\n");
  return devname; 
}


int v4lCapture::dequeue_mmap_buffer(void)
{
  //  fprintf(stderr,"v4lCapture::dequeue_mmap_buffer()\n");
  int frame = dequeued % nbuf;
  int rc;
    
  if ((queued - dequeued) == 0)
    return -1;
  dequeued++;

 retry:
  if ((rc = xioctl(fd,VIDIOCSYNC,buf_v4l+frame)) == -1)
  {
      if (errno == EINTR)
	  goto retry;
  }
  if (-1 == rc)
      return -1;
  return frame;
}

void v4lCapture::clear_mmap_buffers(void)
{
  //  fprintf(stderr,"v4lCapture::clear_mmap_buffers()\n");
  while (queued > dequeued)
    dequeue_mmap_buffer();
  queued   = 0;
  dequeued = 0;
}

bool v4lCapture::is_format_supported(unsigned int fmtid)
{
  //   fprintf(stderr,"v4lCapture::is_format_supported()\n"); 
   if ( fd <= 0 )
   {
     //       fprintf( stderr, "FD is %d\n", fd );
       return false;
   }
   
   if (method == READ_IO_METHOD)
   {

   }
   else if (method == MMAP_IO_METHOD)
   {
       if (queued != dequeued)
       {
	 /*
	   fprintf(stderr,"v4l: found queued buffers (%d %d)\n",
		   queued, dequeued);
	 */
       }
       clear_mmap_buffers();
       buf_v4l[0].frame  = 0;
       buf_v4l[0].width  = capability.minwidth;
       buf_v4l[0].height = capability.minheight;
       buf_v4l[0].format = fmtid;
       
       if (buf_v4l[0].format == 0)
       {
	   return false;
       }
       if (queue_mmap_buffer() == -1)
       {
	   clear_mmap_buffers();
	   return false;
       }
       if (dequeue_mmap_buffer() == -1)
       {
	   clear_mmap_buffers();
	   return false;
       }
       clear_mmap_buffers();
   }
       
   return true;
}



/* capture */
int v4lCapture::setFormat(PixmapFormat& in_fmt)
{
  fprintf(stderr, "v4lCapture::setformat()\n");
  in_fmt.print();

  unsigned int width  = in_fmt.width();
  unsigned int height = in_fmt.height();

  if ((int)width > capability.maxwidth)
  {
      width = capability.maxwidth;
  }
  if ((int)height > capability.maxheight)
  {
      height = capability.maxheight;
  }
    
  if (method == READ_IO_METHOD)
  {
    curr_fmt.setFormat(in_fmt.id(), width, height);

    read_win.width  = curr_fmt.width();
    read_win.height = curr_fmt.height();
    pict.depth    = curr_fmt.depth();
    pict.palette  = v4l_pix_id[curr_fmt.id()];

    if (xioctl(fd, VIDIOCSPICT, &pict) == -1)
    {
	return -1;
    }
    /* for some reason it returns -1, otherwise the call is functioning */
    //if (-1 == xioctl(fd, VIDIOCSWIN,  &read_win))   return -1;

  }
  else if (method == MMAP_IO_METHOD)
  {

#if 0
    /* does driver supports requested format? */
    if ( is_format_supported(v4l_pix_id[in_fmt.id()]) != true )
	return -1;
#endif

    curr_fmt.setFormat(in_fmt.id(), width, height);

    /* initialize buffers */
    nbuf = mbuf.frames;
    for (unsigned int ii = 0; ii < nbuf; ii++)
    {
      buf_v4l[ii].format  = v4l_pix_id[curr_fmt.id()];
      buf_v4l[ii].frame   = ii;
      buf_v4l[ii].width   = curr_fmt.width();
      buf_v4l[ii].height  = curr_fmt.height();

      buf_me[ii].fmt      = curr_fmt;
      buf_me[ii].data_ptr = mmap + mbuf.offsets[ii];
    }

  }

  in_fmt = curr_fmt;
  return 0;
}



int v4lCapture::queue_mmap_buffer(void)
{
  //  fprintf(stderr, "v4lCapture::queue_mmap_buffer()\n");
  int frame = queued % nbuf;
  int rc    = xioctl(fd,VIDIOCMCAPTURE,buf_v4l+frame);
  if (rc == 0)
  {
    //    fprintf(stderr,"v4l: buffer queued \n");
    queued++;
  }
  return rc;
}

int v4lCapture::startVideo(int fps, unsigned int buffers)
{
  fprintf(stderr, "v4lCapture::startvideo()\n");

  fps         = fps;
  frame_count = 0;
  start       = timestamp();

  if (method == MMAP_IO_METHOD)
  {
    nbuf = nbuf > buffers ? buffers : nbuf;
    while (queued - dequeued < nbuf && queue_mmap_buffer() == 0);
  }
  else if (method == READ_IO_METHOD)
  {


  }
  return fd;
}


struct VideoFrame* v4lCapture::nextFrame()
{
  //  fprintf(stderr, "v4lCapture::nextframe()\n");

  struct VideoFrame *video_frame = NULL;
  //  int rc,size;
  int frame = 0;


  if (method == READ_IO_METHOD)
  {
    //    fprintf(stderr, "v4lCapture::nextframe() - READ_IO_METHOD \n");

    /* pict parameters set in setformat()
    pict.depth   = curr_fmt.depth();
    pict.palette = v4l_pix_id[curr_fmt.id()];
    xioctl(fd, VIDIOCSPICT, &pict);
    */
    /* read_win parameters set in setformat()
    read_win.width  = curr_fmt.width();
    read_win.height = curr_fmt.height();
    */
    xioctl(fd, VIDIOCSWIN,  &read_win);

    video_frame = new VideoFrame();
    if ( video_frame == NULL ) return NULL;
    video_frame->data_ptr = new unsigned char[curr_fmt.size()];
    if ( video_frame->data_ptr == NULL )
    {
	delete video_frame;
	return NULL;
    }
    video_frame->fmt = curr_fmt;
    if ((int)curr_fmt.size() != read(fd, video_frame->data_ptr, curr_fmt.size()))
    {
	fprintf(stderr, "v4lCapture: READ %d failed \n", curr_fmt.size());
	delete video_frame->data_ptr;
	delete video_frame;
	return NULL;
    }
    video_frame->seqNo = frame_count++;
    video_frame->msec_ts = timestamp() - start;

    return video_frame;
  }
  else if (method == MMAP_IO_METHOD)
  {
    while (queued - dequeued < nbuf && queue_mmap_buffer() == 0);
    frame = dequeue_mmap_buffer();
    if (frame == -1) return NULL;
    buf_me[frame].msec_ts = timestamp() - start;
    buf_me[frame].seqNo   = frame_count++;
//  return &buf_me[frame];

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
    
  return video_frame;
}


void v4lCapture::stopVideo()
{
  fprintf(stderr, "v4lCapture::stopvideo()\n");

  fps = 0;
  if (method == MMAP_IO_METHOD)
  {
    clear_mmap_buffers();
  }
}


int v4lCapture::close()
{
  fprintf(stderr, "v4lCapture::close(%d)\n",fd);


  if (fd==-1) {
    return -1;
  }
    
  if (method == MMAP_IO_METHOD && mmap != MAP_FAILED)
  {
    fprintf(stderr, "unmapping the buffers \n");
    clear_mmap_buffers();
    munmap(mmap,mbuf.size);
    free(buf_v4l);
    delete [] buf_me;
    buf_v4l = NULL;
    buf_me  = NULL;
    nbuf    = 0;
    mmap    = (unsigned char*)MAP_FAILED;
  }
  else
  {
      fprintf(stderr, "closing using READ method \n");
  }

  if (::close(fd)<0)
  {
    perror("close failed");
    return(fd = -1);
  }
  fd = -1;
  return (0);
}


void v4lCapture::init_palette_table()
{
  v4l_pix_id[PixmapFormat::PIXFMT_NONE]     = 0;

  v4l_pix_id[PixmapFormat::PIXFMT_RGB15_LE] = VIDEO_PALETTE_RGB555;
  v4l_pix_id[PixmapFormat::PIXFMT_RGB16_LE] = VIDEO_PALETTE_RGB565;
  v4l_pix_id[PixmapFormat::PIXFMT_RGB15_BE] = VIDEO_PALETTE_RGB555;
  v4l_pix_id[PixmapFormat::PIXFMT_RGB16_BE] = VIDEO_PALETTE_RGB565;

  v4l_pix_id[PixmapFormat::PIXFMT_BGR24]    = VIDEO_PALETTE_RGB24;
  v4l_pix_id[PixmapFormat::PIXFMT_BGR32]    = VIDEO_PALETTE_RGB32;
  v4l_pix_id[PixmapFormat::PIXFMT_RGB24]    = VIDEO_PALETTE_RGB24;
  v4l_pix_id[PixmapFormat::PIXFMT_RGB32]    = VIDEO_PALETTE_RGB32;

  v4l_pix_id[PixmapFormat::PIXFMT_YUYV]     = VIDEO_PALETTE_YUV422;
  v4l_pix_id[PixmapFormat::PIXFMT_YUV422P]  = VIDEO_PALETTE_YUV422P;
  v4l_pix_id[PixmapFormat::PIXFMT_YUV420P]  = VIDEO_PALETTE_YUV420P;
  v4l_pix_id[PixmapFormat::PIXFMT_UYVY]     = VIDEO_PALETTE_UYVY;

  std::map<PixmapFormat::PixelFormat,unsigned short>::iterator it;
  for(it = v4l_pix_id.begin(); it != v4l_pix_id.end(); it++)
  {
    ptt_pix_id[ it->second ] = it->first;
  }
  fprintf(stderr, "ptt_pix_id.size() = % d\n", ptt_pix_id.size());
}

#include <sys/time.h>
unsigned int v4lCapture::timestamp()
{
    struct timeval tv;
    gettimeofday(&tv,NULL);
    return ((tv.tv_sec * 1000000 + tv.tv_usec) / 1000);
}

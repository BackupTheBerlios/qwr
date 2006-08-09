/*
 * video for linux 2 interface
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

#include"v4lCapture.h"

std::map<PixmapFormat::PixelFormat, unsigned short> v4lCapture::v4l_pix_id;
std::map<unsigned short, PixmapFormat::PixelFormat> v4lCapture::ptt_pix_id;

int v4lCapture::xioctl(int fd, int cmd, void *arg)
{
  int rc;

  rc = ioctl(fd,cmd,arg);
  if (0 == rc)
    return 0;
  fprintf(stderr,"v4lCapture::xioctl: error (%d,%d)-> %s\n",fd, cmd, (rc == 0) ? "ok" : strerror(errno));
  return rc;
}


v4lCapture::v4lCapture()
//    : fd(0)
{
  fprintf(stderr, "v4lCapture::v4lCapture()\n");
  devname = "/dev/video0";
  init_palette_table();
  fd = 0;
  queue  = 0;
  waiton = 0;
}


v4lCapture::~v4lCapture()
{
  fprintf(stderr, "v4lCapture::~v4lCapture()\n");

   
}

void v4lCapture::init()
{
  fprintf(stderr, "v4lCapture::init()\n");
  mmap = (unsigned char*)MAP_FAILED;
  get_device_capabilities();
}


void v4lCapture::get_device_capabilities()
{
  fprintf(stderr, "v4lCapture::get_device_capabilities()\n");

  if (capability.type & VID_TYPE_CAPTURE)
    fprintf(stderr,"size    : %dx%d => %dx%d\n",
            capability.minwidth,capability.minheight,
            capability.maxwidth,capability.maxheight);
}

void v4lCapture::setPictureParams(int colour, int brightness,
                                  int hue,    int contrast)
{
  fprintf(stderr, "v4lCapture::setPictureParams() \n");

  xioctl(fd,VIDIOCGPICT,&pict);

  pict.colour     = colour;
  pict.brightness = brightness;
  pict.hue        = hue;
  pict.contrast   = contrast;
  xioctl(fd, VIDIOCSPICT, &pict);
/*
  xioctl(fd, VIDIOCGPICT, &pict);

  fprintf(stderr,
  "picture : brightness=%d hue=%d colour=%d contrast=%d\n",
  pict.brightness, pict.hue,
  pict.colour, pict.contrast);
  fprintf(stderr,
  "picture : whiteness=%d depth=%d palette=%d\n",
  pict.whiteness, pict.depth, pict.palette);
*/
}


/* open/close */
int  v4lCapture::open(char *device)
{
  fprintf(stderr, "v4lCapture::open()\n");

  if (fd != 0) return fd;

  struct stat st;

  if (-1 == (fd = ::open("/dev/video0", O_RDWR)))
  {
    fprintf(stderr, "::open() - failed\n");
    goto err;
  }
  if (-1 == fstat(fd,&st))
  {
    goto err;
  }

  if (!S_ISCHR(st.st_mode))
  {
    goto err;
  }
  if (major(st.st_rdev) != 81)
  {
    goto err;
  }
  fcntl(fd,F_SETFD,FD_CLOEXEC);
/////////////////// return fd;
  if (-1 == ioctl(fd,VIDIOCGCAP,&capability))
  {
    goto err;
  }
  fprintf(stderr,"size    : %dx%d => %dx%d\n",
          capability.minwidth,capability.minheight,
          capability.maxwidth,capability.maxheight);


  /* picture parameters */
  setPictureParams(32768, 32256, 65535, 34816);

//    v4l_buffer_map(h);
//    int i;
    
  if ((method) && (0 == xioctl(fd,VIDIOCGMBUF,&mbuf)))
  {
    fprintf(stderr,"mbuf: size=%d frames=%d\n", mbuf.size, mbuf.frames);
    mmap = (unsigned char*) ::mmap(0, mbuf.size, PROT_READ|PROT_WRITE,
    MAP_SHARED, fd, 0);
    if (MAP_FAILED == mmap) perror("mmap");
  }
  else
  {
    mmap = (unsigned char*)MAP_FAILED;
  }
  if (MAP_FAILED != mmap)
  {
    fprintf(stderr,"  v4l: using mapped buffers for capture\n");
	// method = 0;
    nbuf = mbuf.frames;

    buf_v4l = (struct video_mmap*)malloc(nbuf * sizeof(struct video_mmap));
    memset(buf_v4l,0,nbuf * sizeof(struct video_mmap));

    buf_me = new VideoFrame[nbuf];
  }
  else
  {
    fprintf(stderr,"  v4l: using read() for capture\n");
	// method = 1;
  }

  return 0;

 err:
     fprintf(stderr, "v4lCapture::open() - failed!!!\n");
 if (-1 != fd)
  ::close(fd);
 return -1;
}




/* attributes */
char* v4lCapture::get_devname()
{
  fprintf(stderr, "v4lCapture::get_devname()\n");
  return devname; 
}

int v4lCapture::can_capture()
{
  fprintf(stderr, "v4lCapture::can_capture()\n");
  int ret = 0;

  if (capability.type & VID_TYPE_CAPTURE)
    ret = 1;
  return ret;
}

int v4lCapture::mm_waiton(void)
{
  //  fprintf(stderr,"v4lCapture::mm_waiton()\n");
  int frame = waiton % nbuf;
  int rc;
    
  if (0 == queue - waiton)
    return -1;
  waiton++;

 retry:
     if (-1 == (rc = xioctl(fd,VIDIOCSYNC,buf_v4l+frame)))
     {
       if (errno == EINTR)
         goto retry;
     }
     if (-1 == rc)
       return -1;
     return frame;
}

void v4lCapture::mm_clear(void)
{
  fprintf(stderr,"v4lCapture::mm_clear()\n");
  while (queue > waiton)
    mm_waiton();
  queue  = 0;
  waiton = 0;
}

bool v4lCapture::is_format_supported(unsigned int fmtid)
{
  fprintf(stderr,"v4lCapture::is_format_supported()\n");

  buf_v4l[0].frame  = 0;
  buf_v4l[0].width  = capability.minwidth;
  buf_v4l[0].height = capability.minheight;
  buf_v4l[0].format = fmtid;

  if (buf_v4l[0].format == 0)
  {
    return false;
  }
  if (queue_buffer() == -1)
  {
    mm_clear();
    return false;
  }
  if (mm_waiton() == -1)
  {
    mm_clear();
    return false;
  }

  fprintf(stderr, "format supported \n");
  mm_clear();
  return true;
}



int v4lCapture::mm_setparams(PixmapFormat& fmt)
{
  fprintf(stderr,"v4lCapture::mm_setparams()\n");

  unsigned int width  = fmt.width();
  unsigned int height = fmt.height();

  if (mbuf.frames < 1)
    return -1;
    
  /* verify parameters */
  xioctl(fd,VIDIOCGCAP,&capability);
  if (fmt.width() > capability.maxwidth)
  {
    width = capability.maxwidth;
  }
  if (fmt.height() > capability.maxheight)
  {
    height = capability.maxheight;
  }
  fprintf(stderr,"v4lCapture: sizes %dx%d \n", width,height);

  fmt = curr_fmt.setFormat(fmt.id(), width, height);

#define DO_FORMAT_CHACK 1
#ifdef DO_FORMAT_CHACK
    /* does driver supports requested format? */
    if ( is_format_supported(v4l_pix_id[curr_fmt.id()]) != true )
	return -1;
#endif


    /* initialize buffers */
    nbuf = mbuf.frames;
    for (int ii = 0; ii < nbuf; ii++)
    {
      buf_v4l[ii].format  = v4l_pix_id[curr_fmt.id()];
      buf_v4l[ii].frame   = ii;
      buf_v4l[ii].width   = curr_fmt.width();
      buf_v4l[ii].height  = curr_fmt.height();

      buf_me[ii].fmt      = curr_fmt;
      buf_me[ii].data_ptr = mmap + mbuf.offsets[ii];
    }
    return 0;
}




/* capture */
int v4lCapture::setformat(PixmapFormat& in_fmt)
{
  fprintf(stderr, "v4lCapture::setformat()\n");

  in_fmt.print();

  int rc;
  unsigned int width  = in_fmt.width();
  unsigned int height = in_fmt.height();

    
  if (method == 0) // = ??
  {
//	rc = read_setformat(in_fmt);

    xioctl(fd,VIDIOCGCAP,&capability);
    if (in_fmt.width() > capability.maxwidth)
    {
      width = capability.maxwidth;
    }
    if (in_fmt.height() > capability.maxheight)
    {
      height = capability.maxheight;
    }
    
    rd_win.width  = width;
    rd_win.height = height;
    rd_win.x = 0;
    rd_win.y = 0;

    curr_fmt = in_fmt.setFormat(in_fmt.id(), width, height);
	
    pict.depth   = 12; //in_fmt_depth YUV420P
    pict.palette = VIDEO_PALETTE_YUV420P;
    if (-1 == xioctl(fd, VIDIOCSPICT, &pict))
      return -1;
    if (-1 == xioctl(fd, VIDIOCSWIN,  &rd_win))
      return -1;

    in_fmt   = curr_fmt.setFormat(in_fmt.id(), width, height);
    rd_fmt   = curr_fmt;
    return 0;
  }
  else
  {
    if (queue != waiton)
    {
      fprintf(stderr,"v4l:setformat: found queued buffers (%d %d)\n",
              queue, waiton);
    }
    mm_clear();
    rc = mm_setparams(in_fmt);
  }
  in_fmt = curr_fmt;

  return rc;
}



int v4lCapture::queue_buffer(void)
{
  //  fprintf(stderr, "v4lCapture::queue_buffer()\n");
  int frame = queue % nbuf;
  int rc;

  if (queue - waiton != 0)
  {
    //    fprintf(stderr,"v4l: waiting for a free buffer\n");
//	return -1;
  }


  rc = xioctl(fd,VIDIOCMCAPTURE,buf_v4l+frame);
  if (0 == rc)
  {
    //    fprintf(stderr,"v4l: buffer queued \n");
    queue++;
  }
  return rc;
}


void v4lCapture::queue_all(void)
{
  //  fprintf(stderr, "v4lCapture::queue_all()\n");
  for (;;)
  {
    if (queue - waiton >= nbuf)
      return;
    if (0 != queue_buffer())
      return;
  }
}

int v4lCapture::startvideo(int fps, unsigned int buffers)
{
  fprintf(stderr, "v4lCapture::startvideo()\n");

  fps         = fps;
  first       = 1;
  frame_count = 0;
  start       = timestamp();

  if (method == 1)
  {
    if (nbuf > buffers)
      nbuf = buffers;
    queue_all();
  }
  return fd;
}



struct VideoFrame* v4lCapture::nextframe()
{
  //  fprintf(stderr, "v4lCapture::nextframe()\n");

  struct VideoFrame *video_frame = NULL;
  //  int rc,size;
  int frame = 0;


  if (method == 0)
  {
//	video_frame = read_getframe();
    if (video_frame == NULL) return NULL;
    video_frame->fmt              = curr_fmt;
/*
    pict.depth   = 12; //in_fmt_depth YUV420P
    pict.palette = VIDEO_PALETTE_YUV420P;
*/
    xioctl(fd, VIDIOCSPICT, &pict);
    xioctl(fd, VIDIOCSWIN,  &rd_win);
    video_frame->data_ptr = (unsigned char*)malloc(curr_fmt.size());
    if ( video_frame->data_ptr == NULL )
    {
      return NULL;
    }
    if (curr_fmt.size() != read(fd, video_frame, curr_fmt.size()))
    {
      return NULL;
    }
    return video_frame;
  }
  else
  {
    queue_all();
    frame = mm_waiton();
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


void v4lCapture::stopvideo()
{
  fprintf(stderr, "v4lCapture::stopvideo()\n");

  fps = 0;
  if (method == 1)
  {
    mm_clear();
  }
}


int v4lCapture::close()
{
  fprintf(stderr, "v4lCapture::close()\n");

//v4l_buffer_unmap();
  if (fd==0 || fd==-1)
  {
    fd=0;
    return -1;
  }
    
  if (MAP_FAILED != mmap)
  {
    fprintf(stderr, "unmapping the buffers \n");
    mm_clear();
    munmap(mmap,mbuf.size);
    free(buf_v4l);
    delete [] buf_me;
//	free(buf_me);
    buf_v4l = NULL;
    buf_me  = NULL;
    nbuf    = 0;
    mmap    = (unsigned char*)MAP_FAILED;
  }
  else
  {
	//method = 0; use_read
  }

  int rc = ::close(fd);
  fprintf(stderr, "close rc = %d \n", rc);
  fd = 0;
  return rc;
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

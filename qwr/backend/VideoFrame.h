#ifndef VIDEO_FRAME_H
#define VIDEO_FRAME_H


#include"pixmapFormat.h"

struct VideoFrame
{
    VideoFrame() : fmt(PixmapFormat::PIXFMT_YUV420P,0,0) {};
    unsigned char* data()       {return data_ptr;};
    unsigned int   time_stamp() {return msec_ts;};
    unsigned int         seqNo;
    PixmapFormat         fmt;
    unsigned int         msec_ts;
    unsigned char*       data_ptr;
};


#endif // VIDEO_FRAME_H

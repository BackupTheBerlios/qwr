#ifndef VIDEO_FRAME_H
#define VIDEO_FRAME_H

#include"video-struct.h"

#ifdef __cplusplus
extern "C" {
#endif


struct VideoFrame
{
    size_t               length;
    struct ng_video_fmt  fmt;
    size_t               size;
    unsigned char       *data_ptr;
    unsigned char        data[1];
};



#ifdef __cplusplus
};
#endif


#endif // VIDEO_FRAME_H

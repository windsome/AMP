#ifndef __VIDEO_OUT_H__
#define __VIDEO_OUT_H__

#include "output.h"

class VideoOuter : public WindOuter{
public:
    virtual ~VideoOuter () {}
    virtual int ShowPicture (AVFrame *pFrame) = 0;
};

class VideoPlay : public OutputPlay {
public:
    void ThreadEntry ();
};

#endif

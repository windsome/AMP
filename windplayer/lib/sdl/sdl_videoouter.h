#ifndef __SDL_VIDEO_OUTER_H__
#define __SDL_VIDEO_OUTER_H__

#include "sdl_abstract.h"
#include "videoout.h"

class SdlVideoOuter : public VideoOuter {
public:
    SdlVideoOuter (SdlAbstract* stub);
    ~SdlVideoOuter ();
    int ShowPicture (AVFrame *pFrame);

private:
    SdlAbstract* mpSdlStub;
};

#endif

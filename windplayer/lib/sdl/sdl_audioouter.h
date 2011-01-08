#ifndef __SDL_AUDIO_OUTER_H__
#define __SDL_AUDIO_OUTER_H__

#include "sdl_abstract.h"
#include "audioout.h"

class SdlAudioOuter : public AudioOuter {
public:
    SdlAudioOuter (SdlAbstract* stub);
    ~SdlAudioOuter ();
    virtual int PlaySound (unsigned char* buffer, int len);

private:
    SdlAbstract* mpSdlStub;
};

#endif

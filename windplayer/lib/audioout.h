#ifndef __AUDIO_OUT_H__
#define __AUDIO_OUT_H__

#include "output.h"

class AudioOuter : public WindOuter{
public:
    virtual ~AudioOuter () {}
    virtual int PlaySound (unsigned char* buffer, int len) = 0;
};

class AudioPlay : public OutputPlay {
public:
    void ThreadEntry ();
};

#endif

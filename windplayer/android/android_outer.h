#ifndef __ANDROID_OUTER_H__
#define __ANDROID_OUTER_H__

#include "audioout.h"
#include "videoout.h"
#include "android_audio.h"
#include "android_video.h"

class AndroidAudioOuter : public AudioOuter {
public:
    AndroidAudioOuter (AndroidAudioOutput* stub);
    ~AndroidAudioOuter ();
    virtual int PlaySound (unsigned char* buffer, int len);

private:
    AndroidAudioOutput* mpStub;
};

class AndroidVideoOuter : public VideoOuter {
public:
    AndroidVideoOuter (AndroidVideoOutput* stub);
    ~AndroidVideoOuter ();
    int ShowPicture (AVFrame *pFrame);

private:
    AndroidVideoOutput* mpStub;
};


#endif

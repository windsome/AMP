#ifndef __SDL_ABSTRACT_H__
#define __SDL_ABSTRACT_H__

#include <stdio.h>
#include <math.h>
#include "ffmpeg.h"
#include "sdl.h"
#include "decoder.h"
#include "util_lock.h"
#include "../config/stl_config.h"

#define SDL_AUDIO_BUFFER_SIZE 1024
typedef struct _SoundFrame {
    int len;
    int index;
    unsigned char data[AudioDecoder::miBufferSize];
}SoundFrame;

class SdlAbstract {
public:
    SdlAbstract ();
    ~SdlAbstract ();

    //int Init (AVFormatContext *pFormatCtx, int vid, int aid);
    int Init (AVStream *vstream, AVStream *astream);

    int ShowPicture (AVFrame *pFrame);

    int PlaySound (unsigned char* buffer, int len);
    static void AudioCallbackStub (void *userdata, Uint8 *stream, int len);

private:
    void AudioCallback (Uint8 *stream, int len);

private:
    UtilSingleLock  mOperationLock;
    SDL_Surface* mpScreen;
    SDL_Overlay* mpBmp;
    SwsContext *mpImgConvertCtx;
    SwsContext *mpImgCvtTmp;
    uint8_t* mpBufTmp;
    int miWidth;
    int miHeight;

    // for sound play.
    deque<SoundFrame> mDeque;
    SoundFrame mCurrentFrame;
};

#endif

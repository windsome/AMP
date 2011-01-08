#ifndef __ANDROID_VIDEO_H__
#define __ANDROID_VIDEO_H__

#include <stdint.h>
#include <sys/types.h>

// SurfaceFlinger
#include <ui/ISurface.h>
#include <ui/Surface.h>

// interprocess shared memory support
#include <binder/MemoryBase.h>
#include <binder/MemoryHeapBase.h>

// ffmpeg
#include "ffmpeg.h"

class AndroidVideoOutput {
public:
    AndroidVideoOutput ();
    ~AndroidVideoOutput ();

    int  SetDisplaySize (int width, int height);
    int  SetSurface (const android::sp<android::Surface>& surface);
    bool InitCheck (AVStream *vstream);
    bool GetVideoSize (int* w, int* h);
    int  ShowPicture (AVFrame *pFrame);
    /** 
     * reset this object. 
     * if you want to reuse it, you need then call SetSurface (), and InitCheck ().
     * 
     * @return 
     */
    int  Reset ();

private:
    int  InitScaler (AVStream *vstream);
    int  CloseFrameBuffer ();
    int  SetFrameSize (int width, int height);
    int  WriteFrameBuffer (uint8_t* aData, uint32_t aDataLen);

private:
    bool                           mInitialized;
    bool                           mEmulation;
    android::sp<android::ISurface> mSurface;

    // frame buffer support
    static const int                     kBufferCount = 2;
    int                                  mFrameBufferIndex;
    android::sp<android::MemoryHeapBase> mFrameHeap;
    size_t                               mFrameBuffers[kBufferCount];

    // frame and surface size.
    int iVideoDisplayWidth;
    int iVideoDisplayHeight;
    int iVideoWidth;
    int iVideoHeight;
    int iVideoFrameSize;

    SwsContext *mpImgConvertCtx;
};

#endif

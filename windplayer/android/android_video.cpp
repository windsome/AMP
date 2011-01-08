#include "android_video.h"
#include "util_log.h"
#define LOG_TAG "AndroidVideoOutput"

//#define USE_COLOR_CONVERTER 1

namespace android {
class Test {
public:
    static android::sp<android::ISurface> getISurface (const android::sp<android::Surface>& surface) {
        if (surface != NULL) {
            return surface->getISurface ();
        } else {
            ERROR ("surface == NULL!");
            return NULL;
        }
    }
};
};

AndroidVideoOutput::AndroidVideoOutput () {
    mInitialized = false;
    mEmulation = false;
    mSurface = NULL;

    iVideoDisplayWidth = -1;
    iVideoDisplayHeight = -1;
    iVideoWidth = -1;
    iVideoHeight = -1;
    iVideoFrameSize = -1;

    mpImgConvertCtx = NULL;
}
AndroidVideoOutput::~AndroidVideoOutput () {
    CloseFrameBuffer ();
    if (mpImgConvertCtx) {
        sws_freeContext (mpImgConvertCtx);
        mpImgConvertCtx = NULL;
    }
}

int  AndroidVideoOutput::SetDisplaySize (int width, int height) {
    iVideoDisplayWidth = width;
    iVideoDisplayHeight = height;
    return 0;
}
int  AndroidVideoOutput::SetSurface (const android::sp<android::Surface>& surface) {
    if (surface != NULL) {
        android::sp<android::ISurface> isurface = android::Test::getISurface (surface);
        if (isurface != NULL) {
            DEBUG ("get ISurface! isurface = %p", isurface.get());
            mSurface = isurface;
            return 0;
        }
    } else {
        mSurface = NULL;
        return 0;
    }
    return -1;
}
bool AndroidVideoOutput::InitCheck (AVStream *vstream) {
    // release resources if previously initialized
    CloseFrameBuffer();

    #if 0
    // initialize only when we have all the required parameters
    if (iVideoDisplayWidth <=0 || iVideoDisplayHeight <= 0 || iVideoWidth <= 0 || iVideoHeight <= 0) {
        ERROR ("display or frame size error! display(%d, %d), frame(%d, %d)", 
               iVideoDisplayWidth, iVideoDisplayHeight, iVideoWidth, iVideoHeight);
        return false;
    }
    #endif

    if (InitScaler (vstream) < 0) {
        ERROR ("InitScaler fail!");
        return false;
    }

    // copy parameters in case we need to adjust them
    int displayWidth = iVideoDisplayWidth;
    int displayHeight = iVideoDisplayHeight;
    int frameWidth = iVideoWidth;
    int frameHeight = iVideoHeight;
    int frameSize;

    // RGB-565 frames are 2 bytes/pixel
    displayWidth = (displayWidth + 1) & -2;
    displayHeight = (displayHeight + 1) & -2;
    frameWidth = (frameWidth + 1) & -2;
    frameHeight = (frameHeight + 1) & -2;
    frameSize = frameWidth * frameHeight * 2;
    iVideoFrameSize = frameSize;

    // create frame buffer heap and register with surfaceflinger
    mFrameHeap = new android::MemoryHeapBase(frameSize * kBufferCount);
    if (mFrameHeap->heapID() < 0)
    {
        ERROR ("Error creating frame buffer heap!");
        return false;
    }
    DEBUG ("allcate buffers for surface(%p)!(%d, %d, %d, %d)", mSurface.get(), 
           displayWidth, displayHeight, displayWidth, displayHeight);
    android::ISurface::BufferHeap buffers(displayWidth, displayHeight,
                                          displayWidth, displayHeight, android::PIXEL_FORMAT_RGB_565, mFrameHeap);
    DEBUG ("mSurface(%p)->registerBuffers", mSurface.get());
    mSurface->registerBuffers(buffers);

    // create frame buffers
    for (int i = 0; i < kBufferCount; i++)
    {
        mFrameBuffers[i] = i * frameSize;
    }

    #ifdef USE_COLOR_CONVERTER
    // initialize software color converter
    iColorConverter = ColorConvert16::NewL();
    iColorConverter->Init(displayWidth, displayHeight, frameWidth, displayWidth, displayHeight, displayWidth, CCROTATE_NONE);
    iColorConverter->SetMemHeight(frameHeight);
    iColorConverter->SetMode(1);
    #endif

    DEBUG ("video = %d x %d", displayWidth, displayHeight);
    DEBUG ("frame = %d x %d", frameWidth, frameHeight);
    DEBUG ("frame #bytes = %d", frameSize);

    // register frame buffers with SurfaceFlinger
    mFrameBufferIndex = 0;
    mInitialized = true;
    //mPvPlayer->sendEvent(MEDIA_SET_VIDEO_SIZE, iVideoDisplayWidth, iVideoDisplayHeight);

    return mInitialized;
}
int  AndroidVideoOutput::WriteFrameBuffer (uint8_t* aData, uint32_t aDataLen) {
    if (mSurface == NULL) return -1;

    if (++mFrameBufferIndex == kBufferCount) mFrameBufferIndex = 0;

    #ifdef USE_COLOR_CONVERTER
    iColorConverter->Convert(aData, static_cast<uint8_t*>(mFrameHeap->base()) + mFrameBuffers[mFrameBufferIndex]);
    #endif

    // post to SurfaceFlinger
    mSurface->postBuffer(mFrameBuffers[mFrameBufferIndex]);
    return 0;
}
bool AndroidVideoOutput::GetVideoSize (int* w, int* h) {
    *w = iVideoDisplayWidth;
    *h = iVideoDisplayHeight;
    return iVideoDisplayWidth > 0 && iVideoDisplayHeight > 0;
}
int  AndroidVideoOutput::InitScaler (AVStream *vstream) {
    if (mpImgConvertCtx) {
        sws_freeContext (mpImgConvertCtx);
        mpImgConvertCtx = NULL;
    }
#if 0
    if (iVideoDisplayWidth <= 0 || iVideoDisplayHeight <= 0) {
        ERROR ("video display size is invalid! width=%d, height=%d", iVideoDisplayWidth, iVideoDisplayHeight);
        return -1;
    }
#endif
    if (vstream == NULL) {
        ERROR ("vstream == NULL!");
        return -1;
    }

    AVCodecContext *codecCtx = vstream->codec;
    if (codecCtx == NULL) {
        ERROR ("error! codecCtx == NULL!");
        return -1;
    }
    int width = 0, height = 0;
    width = codecCtx->width;
    height = codecCtx->height;
    SetFrameSize (width, height);
    SetDisplaySize (width, height);
    SwsContext *img_convert_ctx = sws_getContext (width, height, codecCtx->pix_fmt,
                                                  iVideoDisplayWidth, iVideoDisplayHeight, PIX_FMT_RGB565,
                                                  SWS_BICUBIC, NULL, NULL, NULL);
    if (img_convert_ctx == NULL) {
        ERROR ("img_convert_ctx == NULL!");
    }
    mpImgConvertCtx = img_convert_ctx;
    return 0;
}
int  AndroidVideoOutput::SetFrameSize (int width, int height) {
    iVideoWidth = width;
    iVideoHeight = height;
    return 0;
}
int  AndroidVideoOutput::CloseFrameBuffer () {
    if (!mInitialized) {
        DEBUG ("has not initialized!");
        return 0;
    }

    mInitialized = false;
    if (mSurface.get())
    {
        DEBUG ("mSurface->unregisterBuffers");
        mSurface->unregisterBuffers();
    }

    // free frame buffers
    DEBUG ("free frame buffers");
    for (int i = 0; i < kBufferCount; i++)
    {
        mFrameBuffers[i] = 0;
    }

    // free heaps
    DEBUG ("free mFrameHeap");
    mFrameHeap.clear();

    #ifdef USE_COLOR_CONVERTER
    // free color converter
    if (iColorConverter != 0)
    {
        DEBUG ("free color converter");
        delete iColorConverter;
        iColorConverter = 0;
    }
    #endif
    return 0;
}
int  AndroidVideoOutput::ShowPicture (AVFrame *pFrame) {
    VERBOSE ("begin");
    if (pFrame == NULL) {
        ERROR ("pFrame == NULL!");
        return -1;
    }

    if (mpImgConvertCtx == NULL) {
        ERROR ("mpImgConvertCtx == NULL!");
        return -1;
    }

    if (mSurface == NULL || mFrameHeap == NULL) {
        ERROR ("mSurface == NULL || mFrameHeap == NULL!");
        return -1;
    }

    if (++mFrameBufferIndex == kBufferCount) mFrameBufferIndex = 0;

    AVPicture pict;
    /* point pict at the queue */
    pict.data[0] = static_cast<uint8_t*>(mFrameHeap->base()) + mFrameBuffers[mFrameBufferIndex];
    pict.linesize[0] = iVideoDisplayWidth;
    VERBOSE ("do scale! linesize(%d, %d, %d, %d), pict (%p, %d)", 
           pFrame->linesize[0], pFrame->linesize[1],  pFrame->linesize[2], pFrame->linesize[3],
           pict.data[0], pict.linesize[0]);
    // Convert the image into YUV format that SDL uses
    int ret = sws_scale (mpImgConvertCtx, pFrame->data, pFrame->linesize, 0, iVideoHeight, pict.data, pict.linesize);
    VERBOSE ("sws_scale return = %d", ret);
    // post to SurfaceFlinger
    VERBOSE ("postBuffer to SurfaceFlinger!");
    mSurface->postBuffer(mFrameBuffers[mFrameBufferIndex]);

    return 0;
}

int  AndroidVideoOutput::Reset () {
    CloseFrameBuffer ();
    if (mpImgConvertCtx) {
        sws_freeContext (mpImgConvertCtx);
        mpImgConvertCtx = NULL;
    }
    return 0;
}

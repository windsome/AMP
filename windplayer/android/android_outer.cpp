#include "android_outer.h"
#include "util_log.h"
#define LOG_TAG "AndroidAudioOuter"

AndroidAudioOuter::AndroidAudioOuter (AndroidAudioOutput* stub) : mpStub(stub) {
    DEBUG ("construct AndroidAudioOuter");
}

AndroidAudioOuter::~AndroidAudioOuter () {
}

int AndroidAudioOuter::PlaySound (unsigned char* buffer, int len) {
    if (mpStub) {
        //return mpStub->PlaySound (buffer, len);
        return 0;
    }
    return -1;
}

/********************************** 
 * video outer.
 **********************************/
#define LOG_TAG "AndroidVideoOuter"
AndroidVideoOuter::AndroidVideoOuter (AndroidVideoOutput* stub) : mpStub(stub) {
    DEBUG ("construct AndroidVideoOuter");
}

AndroidVideoOuter::~AndroidVideoOuter () {
}

int AndroidVideoOuter::ShowPicture (AVFrame *pFrame) {
    if (mpStub)
        return mpStub->ShowPicture (pFrame);
    else 
        return -1;
#if 0
    if (mpStub) {
        void* aData = NULL;
        int aDataLen = 0;
        //return mpStub->ShowPicture (aData, aDataLen);
        return 0;
    }
    return -1;
#endif
}


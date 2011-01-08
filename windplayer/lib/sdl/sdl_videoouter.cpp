#include "sdl_videoouter.h"

SdlVideoOuter::SdlVideoOuter (SdlAbstract* stub) : mpSdlStub(stub) {
    DEBUG ("construct SdlVideoOuter");
}

SdlVideoOuter::~SdlVideoOuter () {
}

int SdlVideoOuter::ShowPicture (AVFrame *pFrame) {
    if (mpSdlStub) {
        return mpSdlStub->ShowPicture (pFrame);
    }
    return -1;
}


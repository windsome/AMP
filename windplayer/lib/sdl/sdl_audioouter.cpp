#include "sdl_audioouter.h"

SdlAudioOuter::SdlAudioOuter (SdlAbstract* stub) : mpSdlStub(stub) {
    DEBUG ("construct SdlAudioOuter");
}

SdlAudioOuter::~SdlAudioOuter () {
}

int SdlAudioOuter::PlaySound (unsigned char* buffer, int len) {
    if (mpSdlStub) {
        return mpSdlStub->PlaySound (buffer, len);
    }
    return -1;
}


#ifndef __ANDROID_WIND_PLAYER_H__
#define __ANDROID_WIND_PLAYER_H__

//#include "windplayer.h"
#include <ui/Surface.h>
class WindPlayer;
class AndroidVideoOutput;
class AndroidAudioOutput;
class AndroidAudioOuter;
class AndroidVideoOuter;

class AndroidWindPlayer {
public:
    AndroidWindPlayer ();
    ~AndroidWindPlayer ();
    int _setVideoSurface (const android::sp<android::Surface>& surface);
    int _setDataSource (const char* filepath);
    int _start ();
    int _stop ();
    int _pause ();
    int _seek (double sec);
    int _release ();
    /** 
     * reset windplayer.
     * if you need reuse this object, you need call _setVideoSurface() & _setDataSource() first.
     * 
     * @return 
     */
    int _reset ();
    int _setVolume (float left, float right);
    int getVideoWidth ();
    int getVideoHeight ();
    bool isPlaying ();
    double getCurrentPosition ();
    double getDuration ();

private:
    WindPlayer* mpPlayer;
    AndroidVideoOutput* mpVput;
    AndroidAudioOutput* mpAput;
    //AndroidOutputStub* mpOutputStub;
    AndroidAudioOuter* mpAouter;
    AndroidVideoOuter* mpVouter;

    int miVideoWidth;
    int miVideoHeight;
};

#endif

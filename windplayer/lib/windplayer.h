#ifndef __WIND_PLAYER_H__
#define __WIND_PLAYER_H__

#include "videoout.h"
#include "audioout.h"
#include "decoder.h"

class WindPlayer {
private:
    WindDecoder* mpDecoder;
    MasterClock* mpClock;
    VideoPlay* mpVplay;
    AudioPlay* mpAplay;
    VideoOuter* mpVouter;
    AudioOuter* mpAouter;

    double mfDuration;
    double mfStartTime;
public:
    WindPlayer ();
    virtual ~WindPlayer ();

    int SetDataSource (const char* filename);
    AVStream* GetVideoStream ();
    AVStream* GetAudioStream ();

    int SetOuter (VideoOuter* vouter, AudioOuter* aouter);

    int Start ();
    int Stop ();
    int Pause (bool bPause);
    int Seek (double sec);

    double GetDuration ();
    double GetPlayingTime ();
    bool   IsPlaying ();
};

#endif

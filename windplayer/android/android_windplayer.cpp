#include "android_windplayer.h"
#include "windplayer.h"
#include "android_audio.h"
#include "android_video.h"
#include "android_outer.h"
#include "util_log.h"

AndroidWindPlayer::AndroidWindPlayer () {
    // init wind decoder.
    WindPlayer* player = new WindPlayer ();
    if (player == NULL) {
        ERROR ("fatal error! create WindPlayer fail!");
        return;
    }

    // init android environment
    AndroidVideoOutput* vput = new AndroidVideoOutput ();
    if (vput == NULL) {
        DEBUG ("warning! create AndroidVideoOutput fail! no video output!");
    }
    AndroidAudioOutput* aput = new AndroidAudioOutput ();
    if (aput == NULL) {
        DEBUG ("warning! create AndroidAudioOutput fail! no sound output!");
    }

    // create video outer & audio outer to play sound & show picture.
    AndroidAudioOuter* aouter = new AndroidAudioOuter (aput);
    if (aouter == NULL) {
        DEBUG ("warning! create AndroidAudioOuter fail! no sound output!");
    }
    AndroidVideoOuter* vouter = new AndroidVideoOuter (vput);
    if (vouter == NULL) {
        DEBUG ("warning! create AndroidVideoOuter fail! no sound output!");
    }
    player->SetOuter (vouter, aouter);

    mpPlayer = player;
    mpVput = vput;
    mpAput = aput;
    mpAouter = aouter;
    mpVouter = vouter;

    miVideoWidth = -1;
    miVideoHeight = -1;
}
AndroidWindPlayer::~AndroidWindPlayer () {
    if (mpPlayer) delete mpPlayer;
    if (mpAouter) delete mpAouter;
    if (mpVouter) delete mpVouter;
    if (mpVput) delete mpVput;
    if (mpAput) delete mpAput;
}

int AndroidWindPlayer::_setVideoSurface (const android::sp<android::Surface>& surface) {
    if (mpVput) {
        return mpVput->SetSurface (surface);
    } else {
        ERROR ("mpVput == NULL!");
        return -1;
    }
}
int AndroidWindPlayer::_setDataSource (const char* filepath) {
    miVideoWidth = -1;
    miVideoHeight = -1;

    if (mpPlayer) {
        int ret = mpPlayer->SetDataSource (filepath);
        // should set up audio parameters!
        AVStream* vstream = mpPlayer->GetVideoStream ();
        if (mpVput) {
            if (!mpVput->InitCheck (vstream)) {
                ERROR ("InitCheck fail!");
            }
        }
        AVCodecContext *codecCtx = vstream->codec;
        if (codecCtx != NULL) {
            miVideoWidth = codecCtx->width;
            miVideoHeight = codecCtx->height;
        } else {
            ERROR ("error! codecCtx == NULL!");
        }
        AVStream* astream = mpPlayer->GetAudioStream ();
        return ret;
    }
    return -1;
}
int AndroidWindPlayer::_start () {
    if (mpPlayer) {
        DEBUG ("start windplayer!");
        return mpPlayer->Start ();
    }
    return -1;
}
int AndroidWindPlayer::_stop () {
    if (mpPlayer) {
        DEBUG ("stop windplayer!");
        return mpPlayer->Stop ();
    }
    return -1;
}
int AndroidWindPlayer::_pause () {
    if (mpPlayer) {
        DEBUG ("pause windplayer!");
        return mpPlayer->Pause (true);
    }
    return -1;
}
int AndroidWindPlayer::_seek (double sec) {
    if (mpPlayer) {
        DEBUG ("seek windplayer!");
        return mpPlayer->Seek (sec);
    }
    return -1;
}
int AndroidWindPlayer::_release () {
    return 0;
}

int AndroidWindPlayer::_reset () {
    if (mpPlayer) {
        mpPlayer->Stop ();
    }
    if (mpVput) {
        mpVput->Reset ();
    }
    if (mpAput) {
        mpAput->Reset ();
    }
    return 0;
}

int AndroidWindPlayer::_setVolume (float left, float right) {
    return 0;
}

int AndroidWindPlayer::getVideoWidth () {
    return miVideoWidth;
}
int AndroidWindPlayer::getVideoHeight () {
    return miVideoHeight;
}
bool AndroidWindPlayer::isPlaying () {
    if (mpPlayer) {
        return mpPlayer->IsPlaying ();
    } else {
        return false;
    }
}

double AndroidWindPlayer::getCurrentPosition () {
    if (mpPlayer) {
        return mpPlayer->GetPlayingTime ();
    } else {
        return 0.0;
    }
}

double AndroidWindPlayer::getDuration () {
    if (mpPlayer) {
        return mpPlayer->GetDuration ();
    } else {
        return 0.0;
    }
}


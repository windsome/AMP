#include "windplayer.h"
#include "util_log.h"

WindPlayer::WindPlayer () {
    mpDecoder = new WindDecoder ();
    mpClock = new MasterClock (MasterClock::CLOCK_AUDIO);
    mpVplay = new VideoPlay ();
    mpAplay = new AudioPlay ();
    mpVouter = NULL;
    mpAouter = NULL;

    mfDuration = 0.0f;
    mfStartTime = 0.0f;
}

WindPlayer::~WindPlayer () {
    if (mpAplay) delete mpAplay;
    if (mpVplay) delete mpVplay;
    if (mpClock) delete mpClock;
    if (mpDecoder) delete mpDecoder;
}

int WindPlayer::SetDataSource (const char* filename) {
    if (mpDecoder == NULL) {
        ERROR ("mpDecoder == NULL!");
        return -1;
    }

    DEBUG ("decoder->SetDataSource()");
    WindDecoder* decoder = mpDecoder;
    decoder->SetDataSource (filename);
    AVFormatContext* formatCtx = decoder->GetFormatContext ();
    if (formatCtx == NULL) {
        ERROR ("formatCtx == NULL");
        return -1;
    }

    DEBUG ("reset clock!");
    mpClock->Reset ();

    // get duration & start_time
    double duration = formatCtx->duration / AV_TIME_BASE;
    double start_time = formatCtx->start_time / AV_TIME_BASE;
    DEBUG ("duration=%f, start_time=%f", duration, start_time);
    mfDuration = duration;
    mfStartTime = start_time;
    int vid = decoder->GetVideoStreamIndex ();
    int aid = decoder->GetAudioStreamIndex ();

    // create video play thread to play video.
    if (vid >= 0) {
        mpVplay->SetDecoder (decoder);
        AVStream* stream = NULL;
        if (formatCtx != NULL && vid >= 0) {
            stream = formatCtx->streams[vid];
            //DEBUG ("video: time_base:den=%d, num=%d", stream->time_base.den, stream->time_base.num);
        }
        mpVplay->SetStream (stream);
        mpVplay->SetMasterClock (mpClock);
    }
    
    // create audio play thread to play audio.
    if (aid >= 0) {
        mpAplay->SetDecoder (decoder);
        AVStream* stream = NULL;
        if (formatCtx != NULL && aid >= 0) {
            stream = formatCtx->streams[aid];
            //DEBUG ("audio: time_base:den=%d, num=%d", stream->time_base.den, stream->time_base.num);
        }
        mpAplay->SetStream (stream);
        mpAplay->SetMasterClock (mpClock);
    }
    return 0;
}

AVStream* WindPlayer::GetVideoStream () {
    if (mpDecoder == NULL) {
        ERROR ("mpDecoder == NULL");
        return NULL;
    }

    AVFormatContext* formatCtx = mpDecoder->GetFormatContext ();
    if (formatCtx == NULL) {
        ERROR ("formatCtx == NULL");
        return NULL;
    }

    int vid = mpDecoder->GetVideoStreamIndex ();
    if (vid >= 0) {
        return formatCtx->streams[vid];
    }
    return NULL;
}

AVStream* WindPlayer::GetAudioStream () {
    if (mpDecoder == NULL) {
        ERROR ("mpDecoder == NULL");
        return NULL;
    }

    AVFormatContext* formatCtx = mpDecoder->GetFormatContext ();
    if (formatCtx == NULL) {
        ERROR ("formatCtx == NULL");
        return NULL;
    }

    int aid = mpDecoder->GetAudioStreamIndex ();
    if (aid >= 0) {
        return formatCtx->streams[aid];
    }
    return NULL;
}

int WindPlayer::SetOuter (VideoOuter* vouter, AudioOuter* aouter) {
    mpVouter = vouter;
    mpAouter = aouter;
    if (mpVplay) mpVplay->SetOuter (mpVouter);
    if (mpAplay) mpAplay->SetOuter (mpAouter);
    return 0;
}

int WindPlayer::Start () {
    DEBUG ("reset clock");
    if (mpClock) mpClock->Reset ();
    DEBUG ("start decoder");
    if (mpDecoder) mpDecoder->Start ();
    DEBUG ("start vplay");
    if (mpVplay) mpVplay->Start ();
    DEBUG ("start aplay");
    if (mpAplay) mpAplay->Start ();
    DEBUG ("after start all!");
    return 0;
}

int WindPlayer::Stop () {
    // stop decorder, will unblocking video & audio PacketList.
    DEBUG ("stop decoder");
    if (mpDecoder) mpDecoder->Stop ();
    DEBUG ("stop vplay");
    if (mpVplay) mpVplay->Stop ();
    DEBUG ("stop aplay");
    if (mpAplay) mpAplay->Stop ();
    DEBUG ("stoped all!");
    return 0;
}

int WindPlayer::Pause (bool bPause) {
    if (bPause) {
        DEBUG ("stop vplay");
        if (mpVplay) mpVplay->Stop ();
        DEBUG ("stop aplay");
        if (mpAplay) mpAplay->Stop ();
        DEBUG ("stop clock");
        if (mpClock) mpClock->Stop ();
    } else {
        DEBUG ("restart clock");
        if (mpClock) mpClock->Start ();
        DEBUG ("start vplay");
        if (mpVplay) mpVplay->Start ();
        DEBUG ("start aplay");
        if (mpAplay) mpAplay->Start ();
    }
    return -1;
}
int WindPlayer::Seek (double sec) {
    DEBUG ("stop vplay");
    if (mpVplay) mpVplay->Stop ();
    DEBUG ("stop aplay");
    if (mpAplay) mpAplay->Stop ();
    if (mpClock) mpClock->Reset ();
    if (mpDecoder) mpDecoder->Seek (sec);
    DEBUG ("start vplay");
    if (mpVplay) mpVplay->Start ();
    DEBUG ("start aplay");
    if (mpAplay) mpAplay->Start ();
    return 0;
}

double WindPlayer::GetDuration () {
    return mfDuration;
}
double WindPlayer::GetPlayingTime () {
    double curr = 0.0;
    if (mpClock) {
        curr = mpClock->GetCurrentClock ();
    }
    return curr - mfStartTime;
}
bool   WindPlayer::IsPlaying () {
    bool bVplay = false;
    bool bAplay = false;
    if (mpVplay) bVplay = mpVplay->ThreadIsRunning();
    if (mpAplay) bAplay = mpAplay->ThreadIsRunning();
    return (bVplay || bAplay);
}

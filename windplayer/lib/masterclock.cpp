#include "masterclock.h"

MasterClock::MasterClock (ClockType type, double pts, double pts_time) : mOperationLock("MasterClock") {
    Init ();
    switch (type) {
    case CLOCK_EXTERN:
        UseExternClock (pts, pts_time);
        break;
    case CLOCK_VIDEO:
        UseVideoClock (pts, pts_time);
        break;
    case CLOCK_AUDIO:
        UseAudioClock (pts, pts_time);
        break;
    default:
        ERROR ("you should not reach here!");
        break;
    }
}
MasterClock::~MasterClock () {
}

void MasterClock::Reset () {
    mOperationLock.Lock ();
    Init ();
    mOperationLock.Unlock ();
}

void MasterClock::SetOriginClock (double pts, double pts_time, ClockType type) {
    DEBUG ("pts=%f, pts_time=%f, type=%d", pts, pts_time, (int)type);
    mOperationLock.Lock ();
    if (type == CLOCK_NOT_CHANGE) type = meType;
    switch (type) {
    case CLOCK_EXTERN:
        UseExternClock (pts, pts_time);
        break;
    case CLOCK_VIDEO:
        UseVideoClock (pts, pts_time);
        break;
    case CLOCK_AUDIO:
        UseAudioClock (pts, pts_time);
        break;
    default:
        ERROR ("you should not reach here!");
        break;
    }
    mOperationLock.Unlock ();
}

void MasterClock::Stop () {
    mOperationLock.Lock ();
    mbStop = true;
    mfStopTime = SystemTime ();
    mOperationLock.Unlock ();
}

void MasterClock::Start () {
    mOperationLock.Lock ();
    mbStop = false;
    if (mfStopTime > 0) {
        double diff = SystemTime () - mfStopTime;
        switch (meType) {
        case CLOCK_EXTERN:
            mfExternOriginTime += diff;
            break;
        case CLOCK_VIDEO:
            mfVideoOriginTime += diff;
            break;
        case CLOCK_AUDIO:
            mfAudioOriginTime += diff;
            break;
        default:
            ERROR ("you should not reach here!");
            break;
        }
    } else {
        ERROR ("you have not stoped before!");
    }

    mOperationLock.Unlock ();
}

double MasterClock::GetCurrentClock () {
    double ret = -1.0;
    mOperationLock.Lock ();
    switch (meType) {
    case CLOCK_EXTERN:
        ret = GetExternClock ();
        break;
    case CLOCK_VIDEO:
        ret = GetVideoClock ();
        break;
    case CLOCK_AUDIO:
        ret = GetAudioClock ();
        break;
    default:
        ERROR ("should not reach here!");
        ret = -1.0;
        break;
    }
    if (ret > 0 && mbStop) {
        ret -= SystemTime () - mfStopTime;
    }
    mOperationLock.Unlock ();
    return ret;
}

void MasterClock::Init () {
    //meType = CLOCK_EXTERN;
    mbStop = false;
    mfStopTime = -1.0;

    mfExternOriginPts = -1.0; // from VideoDecoder::Decode () or AudioDecoder::Decode () returned pts.
    mfExternOriginTime = -1.0; // av_gettime() / 1000000.0

    mfVideoOriginPts = -1.0; // from VideoDecoder::Decode () returned pts
    mfVideoOriginTime = -1.0; // av_gettime() / 1000000.0
    
    mfAudioOriginPts = -1.0; // from AudioDecoder::Decode () returned pts
    mfAudioOriginTime = -1.0; // av_gettime() / 1000000.0
}

void MasterClock::UseExternClock (double pts, double pts_time) {
    Init ();
    meType = CLOCK_EXTERN;
    mfExternOriginPts = pts;
    mfExternOriginTime = pts_time;
}
void MasterClock::UseVideoClock (double pts, double pts_time) {
    Init ();
    meType = CLOCK_VIDEO;
    mfVideoOriginPts = pts;
    mfVideoOriginTime = pts_time;
}
void MasterClock::UseAudioClock (double pts, double pts_time) {
    Init ();
    meType = CLOCK_AUDIO;
    mfAudioOriginPts = pts;
    mfAudioOriginTime = pts_time;
}

double MasterClock::GetExternClock () {
    if (mfExternOriginPts < 0 || mfExternOriginTime < 0) {
        return -1.0;
    } else {
        return SystemTime() - mfExternOriginTime + mfExternOriginPts;
    }
}

double MasterClock::GetVideoClock () {
    if (mfVideoOriginPts < 0 || mfVideoOriginTime < 0) {
        return -1.0;
    } else {
        return SystemTime() - mfVideoOriginTime + mfVideoOriginPts;
    }
}

double MasterClock::GetAudioClock () {
    if (mfAudioOriginPts < 0 || mfAudioOriginTime < 0) {
        return -1.0;
    } else {
        return SystemTime() - mfAudioOriginTime + mfAudioOriginPts;
    }
}

double MasterClock::SystemTime () {
    return av_gettime() / 1000000.0;
}

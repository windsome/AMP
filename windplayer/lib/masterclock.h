#ifndef __MASTER_CLOCK_H__
#define __MASTER_CLOCK_H__

#include "ffmpeg.h"
#include "util_lock.h"

class MasterClock {
public:
    typedef enum _ClockType {
        CLOCK_NOT_CHANGE = -1,
        CLOCK_EXTERN = 0,
        CLOCK_VIDEO,
        CLOCK_AUDIO
    } ClockType;

    static const double AVThresholdSync = 0.01;
    //static const double AVThresholdNoSync = 10.0;
    static const double AVThresholdNoSync = 2.0;

public:
    MasterClock (ClockType type = CLOCK_EXTERN, double pts = -1.0, double pts_time = -1.0);
    ~MasterClock ();

    void Reset ();
    void SetOriginClock (double pts, double pts_time, ClockType type = CLOCK_NOT_CHANGE);
    //void Pause (bool bPause);
    void Stop ();
    void Start ();
    double GetCurrentClock (); //base on pts.

private:
    void Init ();

    void UseExternClock (double pts, double pts_time);
    void UseVideoClock (double pts, double pts_time);
    void UseAudioClock (double pts, double pts_time);
    
    double GetExternClock ();
    double GetVideoClock ();
    double GetAudioClock ();
    double SystemTime ();

private:
    ClockType meType;
    UtilSingleLock  mOperationLock;

    bool mbStop;
    double mfStopTime;

    double mfExternOriginPts;
    double mfExternOriginTime;

    double mfVideoOriginPts;
    double mfVideoOriginTime;
    
    double mfAudioOriginPts;
    double mfAudioOriginTime;
};

#endif

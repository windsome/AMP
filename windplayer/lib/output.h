#ifndef __OUTPUT_PLAY_H__
#define __OUTPUT_PLAY_H__

#include "ffmpeg.h"
#include "decoder.h"
#include "util_lock.h"
#include "util_thread.h"
#include "packetList.h"
#include "masterclock.h"

/** 
 * this is an empty base class, just a interface. 
 * 
 * 
 */
class WindOuter {
};

class OutputPlay : public UtilThread {
public:
    OutputPlay ();
    ~OutputPlay ();

    void SetDecoder (WindDecoder* decoder);
    void SetStream (AVStream* stream);
    //void SetCodecContext (AVCodecContext* ctx);
    void SetOuter (WindOuter* vout);
    void SetMasterClock (MasterClock* clock);

    int Start ();
    int Stop ();
    virtual void ThreadEntry ();

protected:
    WindDecoder* mpDecoder;
    AVStream* mpStream;
    WindOuter* mpOuter;
    MasterClock* mpClock;

    bool mbQuit;
    UtilSingleLock  mOperationLock;
};

#endif

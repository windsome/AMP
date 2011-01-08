/**
 * @file   decoder.h
 * @author windsome <windsome@windsome-laptop>
 * @date   Fri Aug 20 17:21:08 2010
 * 
 * @brief  decoder using ffmpeg. result in a video and a audio PacketQueue.
 * 
 */
#ifndef __DECODER_H__
#define __DECODER_H__

#include "ffmpeg.h"
#include "decoder_impl.h"
#include <stdio.h>
#include <math.h>
#include "util_lock.h"
#include "util_thread.h"
#include "packetList.h"
#include "config/stl_config.h"

class WindDecoder : public UtilThread {
public:
    WindDecoder ();
    ~WindDecoder ();

    int SetDataSource(const char *url);
    int Start ();
    int Stop ();
    int Seek (double pos);

    void ThreadEntry ();

    AVPacket GetVideoPacket ();
    AVPacket GetAudioPacket ();

    // get information, must call after OpenInputFile ().
    AVFormatContext* GetFormatContext ();
    int GetVideoStreamIndex ();
    int GetAudioStreamIndex ();

private:
    int OpenInputFile (string url);
    int Clear ();
    int AvSeekFrame (double pos);

private:
    string          msUrl;
    bool            mbQuit;
    UtilSingleLock  mOperationLock;

    AVFormatContext *mpFormatCtx;
    int             miVideoStream;
    int             miAudioStream;

    PacketQueue     mAudioQueue;
    PacketQueue     mVideoQueue;
};

#endif

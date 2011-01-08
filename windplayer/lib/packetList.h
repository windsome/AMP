/**
 * @file   packetList.h
 * @author windsome <windsome@windsome-laptop>
 * @date   Tue Aug 24 17:10:54 2010
 * 
 * @brief  
 * 
 * 
 */
#ifndef __PACKET_LIST_H__
#define __PACKET_LIST_H__

#include "ffmpeg.h"
#include <stdio.h>
#include "util_lock.h"
#include "config/stl_config.h"

class PacketQueue {
private:
    int miMaxSize, miSize;
    bool mbToFlush;
    list<pair<AVPacket, int> > mPktList;
    UtilSingleLock mLock;
    UtilCond mCondEmpty;
    UtilCond mCondFull;
public:
    PacketQueue (int maxSize);
    ~PacketQueue ();
    
    void      init ();
    AVPacket  get ();
    int       put (AVPacket* pkt, int size = -1);
    void      flush ();
    static const AVPacket mNullPacket;
};

#endif

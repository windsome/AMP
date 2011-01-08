#include "packetList.h"
#include "util_log.h"

const AVPacket PacketQueue::mNullPacket = {0,0,0,0,0,0,0,0,0,0,0};

PacketQueue::PacketQueue (int size) :
    miMaxSize(size), miSize(0), mbToFlush(false), 
    mLock("AVPacket"), mCondEmpty(), mCondFull() {
    DEBUG ("");
    mPktList.clear ();
}

PacketQueue::~PacketQueue () {
}
    
void PacketQueue::init () {
    mLock.Lock ();
    miSize = 0;
    list<pair<AVPacket, int> >::iterator it = mPktList.begin ();
    for(; it != mPktList.end (); it++) {
        av_free_packet (&it->first);
    }
    mPktList.clear ();
    mbToFlush = false;
    mLock.Unlock ();
}

AVPacket PacketQueue::get () {
    AVPacket packet = mNullPacket;
    mLock.Lock ();
    for (;;) {
        if (mbToFlush) {
            mLock.Unlock ();
            DEBUG ("flush return");
            return mNullPacket;
        }
        if (mPktList.size () <= 0) {
            //if (miSize <= 0) {
            mCondEmpty.Wait (mLock.GetMutex());
            VERBOSE ("<<get mCondEmpty signal! miSize=%d", miSize);
        } else {
            break;
        }
    }
    if (mPktList.size () <= 0) {
        ERROR ("concurrent error! should not reach here! miSize=%d, mPktList.size()=%d\n", miSize, mPktList.size());
        mLock.Unlock ();
        return mNullPacket;
    }
    pair<AVPacket, int> item = mPktList.front();
    mPktList.pop_front ();
    packet = item.first;
    miSize -= item.second;
    //DEBUG ("miSize = %d, out queue size=%d", miSize, item.second);
    mCondFull.Signal ();
    mLock.Unlock ();
    return packet;
}

/** 
 * 
 * @param pkt 
 * @param size , if size == -1, <miSize>, <miMaxSize> stands for packet count.
 *               miSize = mPktList.size(). we should add 1 to miSize.
 *               else the size stands for the size of the <pkt>.
 * 
 * @return 
 */
int PacketQueue::put (AVPacket* pkt, int size) {
    if (size == -1) size = 1;

    //todo: first av_dup_packet, will not copy data, why???
    if (av_dup_packet(pkt) < 0) {
        ERROR ("dup packet fail! maybe this could be optimization! no need to dup?\n");
        return -1;
    }

    mLock.Lock ();

    for (;;) {
        if (mbToFlush) {
            mLock.Unlock ();
            av_free_packet(pkt);
            DEBUG ("flush return");
            return -1;
        }
        if (miSize + size > miMaxSize) {
            mCondFull.Wait (mLock.GetMutex());
            VERBOSE (">>get mCondFull signal! miSize=%d, size=%d", miSize, size);
        } else {
            break;
        }
    }

    pair<AVPacket, int> item = make_pair (*pkt, size);
    mPktList.push_back (item);
    miSize += size;
    //DEBUG ("miSize = %d, in queue size=%d", miSize, size);
    mCondEmpty.Signal ();
    mLock.Unlock ();
    return 0;
}
void PacketQueue::flush () {
    mLock.Lock ();
    mbToFlush = true;
    DEBUG ("send mCondFull Signal");
    mCondFull.Signal ();
    DEBUG ("send mCondEmpty Signal");
    mCondEmpty.Signal ();
    DEBUG ("after send out all Signal");
    miSize = 0;
    list<pair<AVPacket, int> >::iterator it = mPktList.begin ();
    int i = 0;
    for(; it != mPktList.end (); it++) {
        //DEBUG ("i=%d, second=%d", i++, it->second);
        av_free_packet (&it->first);
    }
    mPktList.clear ();
    mLock.Unlock ();
}


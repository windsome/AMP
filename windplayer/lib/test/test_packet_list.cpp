/**
 * @file   test_packet_list.cpp
 * @author windsome <windsome@windsome-laptop>
 * @date   Wed Aug 25 12:11:43 2010
 * 
 * @brief  test packetList.cpp
 * 1, test normal Get and Put, use random timeout.
 * 2, test fast Put(100), slow Get(500), will block Put, then test flush, check whether can normal flush.
 * 3, test fast Get(500), slow Put(100), will block Get, then test flush, check whether can normal flush.
 * 
 */
#include "packetList.h"
#include "util_log.h"
#include "util_cmdline.h"
#include <pthread.h>
#include <stdlib.h>

AVPacket flush_pkt;
bool gsToFlush = false;

void *WrapPut(void *ptr) {
    PacketQueue* q2 = (PacketQueue*)ptr;

    unsigned char buf[10] = {'1'};
    av_init_packet(&flush_pkt);
    flush_pkt.data = buf;
    flush_pkt.size = 10;

    int i = 1;
    INFO ("start WrapPut");
    while (1) {
        if (gsToFlush)
            break;
        int a = (i++)%10000;
        sprintf ((char*)flush_pkt.data, "%9d", a);
        DEBUG ("put [%s](%d)", (char*)flush_pkt.data, (i%24)+1);
        q2->put (&flush_pkt, (i%24)+1);
        int timeout = (random() % 30+1) * 10;
        //DEBUG ("put time out:%d", timeout);
        //usleep (timeout*2000);
        usleep (500*1000);

    }
    return NULL;
}

void *WrapGet (void* ptr) {
    PacketQueue* q1 = (PacketQueue*)ptr;

    INFO ("start WrapGet!");
    while (1) {
        if (gsToFlush)
            break;
        AVPacket pkt = q1->get();
        if (pkt.data) {
            DEBUG ("get Data: %s", pkt.data);
        } else {
            DEBUG ("get null");
        }
        int timeout = (random() % 30+1) * 10;
        //DEBUG ("get time out:%d", timeout);
        //usleep (timeout*2000);
        usleep (100*1000);
    }

}

int main (int argc, char* argv[]) {
    PacketQueue q1(100);

    pthread_t mt_Get, mt_Put;
    pthread_attr_t attr;
    pthread_attr_init(&attr);

    if ( pthread_create(&mt_Get, &attr, WrapGet, &q1) ) {
        DEBUG("couldn't create new thread\n");
        return -1;
    }
    if ( pthread_create(&mt_Put, &attr, WrapPut, &q1) ) {
        DEBUG("couldn't create new thread\n");
        return -1;
    }

    INFO ("start main loop!");
    UtilCmdLine cmdline;
    while (1) {
        string cmdstr = cmdline.WaitCmdLine ();
        DEBUG ("get command:%s", cmdstr.c_str());

        if (cmdstr == "quit") {
            break;
        } else if (cmdstr == "flush") {
            DEBUG ("we need to flush!");
            gsToFlush = true;
            q1.flush();
        }
    }
    return 0;
}

#include "decoder.h"
#include "util_log.h"
#include "util_cmdline.h"
#include <pthread.h>
#include <stdlib.h>

int main (int argc, char* argv[]) {
    if(argc < 2) {
        printf ("usage: %s filename\n", argv[0]);
        printf ("   eg: %s /home/media/450k.mp4\n", argv[0]);
        return -1;
    }
    char* filename = argv[1];

    WindDecoder* decoder = new WindDecoder();
    decoder->SetDataSource (filename);

    INFO ("start main loop!");
    UtilCmdLine cmdline;
    while (1) {
        string cmdstr = cmdline.WaitCmdLine ();
        DEBUG ("get command:%s", cmdstr.c_str());

        if (cmdstr == "quit") {
            break;
        } else if (cmdstr == "start") {
            DEBUG ("start decoder!");
            decoder->Start ();
        } else if (cmdstr == "stop") {
            DEBUG ("stop decoder!");
            decoder->Stop ();
        } else if (cmdstr == "seek") {
            DEBUG ("seek decoder!");
            decoder->Seek (10);
        }
    }
    return 0;
}

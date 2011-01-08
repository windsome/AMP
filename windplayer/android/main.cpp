#include "android_windplayer.h"
#include "util_log.h"
#include "util_cmdline.h"

int main (int argc, char* argv[]) {
    const char* filename = NULL;
    if(argc < 2) {
        printf ("usage: %s filename\n", argv[0]);
        printf ("   eg: %s /sdcard/450k.mp4 (if no file specified, use this for test!)\n", argv[0]);

        filename = "/sdcard/450k.mp4";
    } else {
        filename = argv[1];
    }

    AndroidWindPlayer* player = new AndroidWindPlayer ();
    player->_setDataSource (filename);
    INFO ("start main loop!");
    UtilCmdLine cmdline;
    while (1) {
        string cmdstr = cmdline.WaitCmdLine ();
        DEBUG ("get command:%s", cmdstr.c_str());

        if (cmdstr == "quit") {
            break;
        } else if (cmdstr == "start") {
            player->_start ();
        } else if (cmdstr == "stop") {
            player->_stop ();
        } else if (cmdstr == "seek") {
            player->_seek (10.0);
        }
    }
    delete player;
    return 0;
}

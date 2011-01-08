#include "windplayer.h"
#include "sdl_abstract.h"
#include "sdl_videoouter.h"
#include "sdl_audioouter.h"
#include "util_log.h"
#include "util_cmdline.h"

int main (int argc, char* argv[]) {
    const char* filename = NULL;
    if(argc < 2) {
        printf ("usage: %s filename\n", argv[0]);
        printf ("   eg: %s /home/media/450k.mp4 (if no file specified, use this for test!)\n", argv[0]);

        filename = "/home/media/450k.mp4";
    } else {
        filename = argv[1];
    }

    // init wind decoder.
    WindPlayer* player = new WindPlayer ();
    player->SetDataSource (filename);
    AVStream* vstream = player->GetVideoStream ();
    AVStream* astream = player->GetAudioStream ();

    // init SDL
    SdlAbstract* sdl = new SdlAbstract ();
    sdl->Init (vstream, astream);

    // create a video outer to show picture.
    SdlVideoOuter* vouter = new SdlVideoOuter (sdl);

    // create a video outer to show picture.
    SdlAudioOuter* aouter = new SdlAudioOuter (sdl);

    player->SetOuter (vouter, aouter);

    INFO ("start main loop!");
    UtilCmdLine cmdline;
    bool bPause = false;
    while (1) {
        string cmdstr = cmdline.WaitCmdLine ();
        double time = player->GetPlayingTime ();
        DEBUG ("get command:%s, current playing time:%f", cmdstr.c_str(), time);

        if (cmdstr == "quit") {
            break;
        } else if (cmdstr == "start") {
            DEBUG ("start windplayer!");
            player->Start ();
        } else if (cmdstr == "stop") {
            DEBUG ("stop windplayer!");
            player->Stop ();
        } else if (cmdstr == "pause") {
            bPause = !bPause;
            DEBUG ("pause(%d) windplayer!", (int)bPause);
            player->Pause (bPause);
        } else if (cmdstr == "seek") {
            DEBUG ("seek windplayer!");
            player->Seek (10);
        }

    }
    return 0;
}

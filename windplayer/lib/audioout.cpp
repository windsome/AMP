#include <unistd.h>
#include "audioout.h"

void AudioPlay::ThreadEntry () {
    WindDecoder* decoder = mpDecoder;
    AVStream* stream = mpStream;
    AudioOuter* outer = (AudioOuter*)mpOuter;
    MasterClock* clock = mpClock;
    if (decoder == NULL || stream == NULL || outer == NULL || stream->codec == NULL) {
        ERROR ("mpDecoder || mpStream || mpOuter || mpStream->codec== NULL!");
        return;
    }
    AVCodecContext *avctx = stream->codec;

    DEBUG ("start main loop!");

    for(;;) {
        if(mbQuit) {
            break;
        }
        AVPacket avpkt = decoder->GetAudioPacket ();
        if (avpkt.data == 0 || avpkt.size == 0) {
            WARN ("get NULL packet! continue! must be end of stream!!! stop thread!");
            //continue;
            break;
        }

        int ignore_size = 0;
        do {
            uint8_t* frame_buf = AudioDecoder::mBuffer;
            int frame_size = AudioDecoder::miBufferSize;
            int64_t pts = 0;
            int len = AudioDecoder::Decode (avctx, (int16_t*)frame_buf, &frame_size, &avpkt, &pts, ignore_size);
            ignore_size += len;
            if (len < 0) {
                ERROR ("decode frame error!");
                break;
            }
            if (frame_size <= 0) {
                VERBOSE ("no data! begin next frame!");
                break;
            }
        
            if (clock) {
                double curr_pts = clock->GetCurrentClock ();
                double frame_pts = pts * av_q2d (stream->time_base);
                if (curr_pts < 0) {
                    double clock_pts = frame_pts;
                    double clock_time = av_gettime() / 1000000.0;
                    clock->SetOriginClock (clock_pts, clock_time);
                    curr_pts = clock->GetCurrentClock ();
                }
            
                double diff = frame_pts - curr_pts;
                DEBUG ("A:curr_pts=%f, frame_pts=%f, diff=%f", curr_pts, frame_pts, diff);

                if (fabs(diff) < MasterClock::AVThresholdNoSync) {
                    if (diff <= 0) {
                        VERBOSE ("show it at once.");
                    } else {
                        unsigned int usec = diff * 1000 * 1000;
                        VERBOSE ("wait %d usec", usec);
                        usleep (usec);
                    }
                    outer->PlaySound (frame_buf, frame_size);
                } else {
                    if (diff < 0) {
                        WARN ("audio frame_pts far slow than curr_pts, we reset master timer to audio frame_pts");
                        double clock_pts = frame_pts;
                        double clock_time = av_gettime() / 1000000.0;
                        clock->SetOriginClock (clock_pts, clock_time);
                        outer->PlaySound (frame_buf, frame_size);
                    } else {
                        WARN ("no sync! ignore this frame!");
                    }
                }
            } else {
                ERROR ("you have not set Master Clock!!! will not show pictures!");
            }
        } while(1);
        av_free_packet (&avpkt);
    }
    DEBUG ("end of audio out thread!");
}

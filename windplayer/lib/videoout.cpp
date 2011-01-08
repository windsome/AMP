#include <unistd.h>
#include "videoout.h"

void VideoPlay::ThreadEntry () {
    WindDecoder* decoder = mpDecoder;
    AVStream* stream = mpStream;
    VideoOuter* outer = (VideoOuter*)mpOuter;
    MasterClock* clock = mpClock;
    if (decoder == NULL || stream == NULL || outer == NULL || stream->codec == NULL) {
        ERROR ("mpDecoder || mpStream || mpOuter || mpStream->codec== NULL!");
        return;
    }
    AVCodecContext *avctx = stream->codec;

    DEBUG ("start main loop!");

    AVFrame *pFrame = avcodec_alloc_frame();

    for(;;) {
        if(mbQuit) {
            break;
        }
        AVPacket avpkt = decoder->GetVideoPacket ();
        if (avpkt.data == 0 || avpkt.size == 0) {
            WARN ("get NULL packet! continue! must be end of stream!!! stop thread!");
            //continue;
            break;
        }

        int frameFinished;
        int64_t pts;
        int len = VideoDecoder::Decode (avctx, pFrame, &frameFinished, &avpkt, &pts);
        if (len < 0) {
            ERROR ("VideoDecoder::Decode fail! continue to next packet!");
            av_free_packet (&avpkt);
            continue;
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
            DEBUG ("V:curr_pts=%f, frame_pts=%f, diff=%f", curr_pts, frame_pts, diff);

            if (fabs(diff) < MasterClock::AVThresholdNoSync) {
                if (diff <= 0) {
                    VERBOSE ("show it at once.");
                } else {
                    unsigned int usec = diff * 1000 * 1000;
                    VERBOSE ("wait %d usec", usec);
                    usleep (usec);
                }
                outer->ShowPicture (pFrame);
            } else {
                if (diff < 0) {
                    WARN ("video frame_pts far slow than curr_pts, we reset master timer to video frame_pts");
                    double clock_pts = frame_pts;
                    double clock_time = av_gettime() / 1000000.0;
                    clock->SetOriginClock (clock_pts, clock_time);
                    outer->ShowPicture (pFrame);
                } else {
                    WARN ("no sync! ignore this frame!");
                }
            }
        } else {
            ERROR ("you have not set Master Clock!!! will not show pictures!");
        }
        av_free_packet (&avpkt);
    }
    av_free(pFrame);
    DEBUG ("end of video out thread!");
}


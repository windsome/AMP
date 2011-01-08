#include "decoder_impl.h"
#include "util_log.h"

uint8_t AudioDecoder::mBuffer[AudioDecoder::miBufferSize] = {0};
AVRational AudioDecoder::mTimebase = {1, 90000};
int64_t AudioDecoder::mNextPts = 0;
//uint8_t AudioDecoder::mBuffer[(AVCODEC_MAX_AUDIO_FRAME_SIZE * 3) / 2] = {0};
//int AudioDecoder::miBufferSize = (AVCODEC_MAX_AUDIO_FRAME_SIZE * 3) / 2;
void AudioDecoder::SetTimebase (AVRational timebase) {
    mTimebase = timebase;
}

int AudioDecoder::Clear () {
    global_pkt_pts = 0;
    mNextPts = 0;
    return 0;
}

int AudioDecoder::Decode (AVCodecContext *avctx, int16_t *samples, int *frame_size_ptr, AVPacket *avpkt, int64_t* pts, int ignore_size) {
    if (avctx == NULL || avpkt == NULL || pts == NULL || avpkt->data == NULL || avpkt->size == 0 || frame_size_ptr == NULL) {
        ERROR ("parameter error!");
        return -1;
    }
    *pts = 0;

    int len1 = avcodec_decode_audio2 (avctx, samples, frame_size_ptr, avpkt->data+ignore_size, avpkt->size-ignore_size);
    if (len1 < 0) {
        ERROR ("decode audio error!");
        return len1;
    }
    if (*frame_size_ptr <= 0) {
        VERBOSE ("no data yet! need next frame!");
        return len1;
    }
    int64_t currPts = mNextPts;
    int data_size = *frame_size_ptr;
    if (ignore_size == 0)
        currPts = avpkt->pts;
    mNextPts = currPts + data_size / (2 * avctx->channels * avctx->sample_rate) * (mTimebase.den/mTimebase.num);
    //mNextPts = currPts + data_size / (2 * avctx->channels * avctx->sample_rate);
    //DEBUG ("mNextPts=%lld, currPts=%lld, data_size=%d, channels=%d, sample_rate=%d, den=%d, num=%d", mNextPts, currPts, data_size, avctx->channels, avctx->sample_rate, mTimebase.den, mTimebase.num);
    *pts = currPts;
    return len1;
}

uint64_t AudioDecoder::global_pkt_pts = AV_NOPTS_VALUE;

int AudioDecoder::our_get_buffer(struct AVCodecContext *c, AVFrame *pic) {
    int ret = avcodec_default_get_buffer(c, pic);
    uint64_t *pts = (uint64_t*)av_malloc(sizeof(uint64_t));
    *pts = global_pkt_pts;
    //DEBUG ("debug audio pts=%ld", global_pkt_pts);
    pic->opaque = pts;
    return ret;
}
void AudioDecoder::our_release_buffer(struct AVCodecContext *c, AVFrame *pic) {
    if(pic) av_freep(&pic->opaque);
    avcodec_default_release_buffer(c, pic);
}

/**************************************************** 
 * video decoder
 ****************************************************/

int VideoDecoder::Clear () {
    global_pkt_pts = 0;
    return 0;
}

int VideoDecoder::Decode (AVCodecContext *avctx, AVFrame *picture, int *got_picture_ptr, AVPacket *avpkt, int64_t* pts, int ignore_size) {
    if (avctx == NULL || avpkt == NULL || pts == NULL) {
        ERROR ("parameter error!");
        return -1;
    }
    *pts = 0;

    int64_t gotPts = 0;
    global_pkt_pts = avpkt->pts;
    //int len1 = avcodec_decode_video2 (avctx, picture, got_picture_ptr, avpkt);
    int len1 = avcodec_decode_video (avctx, picture, got_picture_ptr, avpkt->data, avpkt->size);

    if(avpkt->dts == AV_NOPTS_VALUE && picture->opaque != NULL && *(uint64_t*)picture->opaque != AV_NOPTS_VALUE) {
        gotPts = *(uint64_t *)picture->opaque;
    } else if(avpkt->dts != AV_NOPTS_VALUE) {
        gotPts = avpkt->dts;
    } else {
        gotPts = 0;
    }
    *pts = gotPts;
    return len1;
}

uint64_t VideoDecoder::global_pkt_pts = AV_NOPTS_VALUE;

int VideoDecoder::our_get_buffer(struct AVCodecContext *c, AVFrame *pic) {
    int ret = avcodec_default_get_buffer(c, pic);
    uint64_t *pts = (uint64_t*)av_malloc(sizeof(uint64_t));
    *pts = global_pkt_pts;
    pic->opaque = pts;
    return ret;
}
void VideoDecoder::our_release_buffer(struct AVCodecContext *c, AVFrame *pic) {
    if(pic) av_freep(&pic->opaque);
    avcodec_default_release_buffer(c, pic);
}

#include "decoder.h"
WindDecoder::WindDecoder () : msUrl(""), mbQuit(false), mOperationLock("WindDecoder"), mpFormatCtx(NULL),
                              miVideoStream(-1), miAudioStream(-1), mAudioQueue(2*1024*1024), mVideoQueue(2*1024*1024) {
    //PacketQueue     mAudioQueue;
    //PacketQueue     mVideoQueue;

    // Register all formats and codecs
    av_register_all();

}
WindDecoder::~WindDecoder () {
}

int WindDecoder::SetDataSource(const char *url) {
    if (url == NULL) {
        ERROR ("url == NULL!");
        return -1;
    }

    mOperationLock.Lock ();
    int ret = Clear ();
    msUrl = url;
    ret = OpenInputFile (msUrl);
    mOperationLock.Unlock ();
    return ret;
}

int WindDecoder::Start () {
    mOperationLock.Lock ();
    mVideoQueue.init ();
    mAudioQueue.init ();
    mbQuit = false;
    if (AvSeekFrame (0) < 0) {
        ERROR ("seek fail!");
    } else {
        ThreadExec ();
    }
    mOperationLock.Unlock ();
    return 0;
}
int WindDecoder::Stop () {
    mOperationLock.Lock ();
    if (mpFormatCtx) {
        mbQuit = true;
        mVideoQueue.flush ();
        mAudioQueue.flush ();
        DEBUG ("begin wait thread!");
        ThreadWait ();
        DEBUG ("end wait thread!");
        if (miVideoStream >= 0) {
            WARN ("todo:clear video decoder!");
        }
        if (miAudioStream >= 0) {
            WARN ("todo:clear audio decoder!");
        }
        //mVideoQueue.init ();
        //mAudioQueue.init ();
        //mbQuit = false;
    }
    mOperationLock.Unlock ();
    return 0;
}
int WindDecoder::Seek (double pos) {
    //pos is seconds based.
    mOperationLock.Lock ();
    mbQuit = true;
    mVideoQueue.flush ();
    mAudioQueue.flush ();
    DEBUG ("begin wait thread!");
    ThreadWait ();
    DEBUG ("end wait thread!");

    mVideoQueue.init ();
    mAudioQueue.init ();
    mbQuit = false;
    if (AvSeekFrame (pos) < 0) {
        ERROR ("seek fail!");
    } else {
        ThreadExec ();
    }
    mOperationLock.Unlock ();
    return 0;
}

int WindDecoder::OpenInputFile (string url) {
    AVFormatContext *pFormatCtx;
    int video_index = -1;
    int audio_index = -1;

    // Open video file
    if(av_open_input_file(&pFormatCtx, url.c_str (), NULL, 0, NULL)!=0) {
        ERROR ("Couldn't open file:%s", url.c_str ());
        return -1;
    }

    // Retrieve stream information
    if(av_find_stream_info(pFormatCtx)<0) {
        ERROR ("Couldn't find stream information!");
        av_close_input_file (pFormatCtx);
        return -1;
    }

    // Dump information about file onto standard error
    dump_format(pFormatCtx, 0, msUrl.c_str (), 0);
  
    // Find the first video stream
    for(int i = 0; i < pFormatCtx->nb_streams; i++) {
        if(pFormatCtx->streams[i]->codec->codec_type==CODEC_TYPE_VIDEO && video_index < 0) {
            video_index=i;
        }
        if(pFormatCtx->streams[i]->codec->codec_type==CODEC_TYPE_AUDIO && audio_index < 0) {
            audio_index=i;
        }
    }

    if (audio_index < 0 && video_index < 0) {
        ERROR ("there is no audio or video stream in the file, quit!");
        av_close_input_file (pFormatCtx);
        return -1;
    }

    // Open audio codec
    if(audio_index >= 0) {
        AVCodecContext *codecCtx = pFormatCtx->streams[audio_index]->codec;
        AVCodec *codec = avcodec_find_decoder(codecCtx->codec_id);
        if(!codec || (avcodec_open(codecCtx, codec) < 0)) {
            ERROR("Unsupported codec!");
            av_close_input_file (pFormatCtx);
            return -1;
        }
        //codecCtx->get_buffer = AudioDecoder::our_get_buffer;
        //codecCtx->release_buffer = AudioDecoder::our_release_buffer;
        AudioDecoder::SetTimebase (pFormatCtx->streams[audio_index]->time_base);
    }

    // Open video codec
    if(video_index >= 0) {
        AVCodecContext *codecCtx = pFormatCtx->streams[video_index]->codec;
        AVCodec *codec = avcodec_find_decoder(codecCtx->codec_id);
        if(!codec || (avcodec_open(codecCtx, codec) < 0)) {
            ERROR("Unsupported codec!");
            av_close_input_file (pFormatCtx);
            return -1;
        }
        codecCtx->get_buffer = VideoDecoder::our_get_buffer;
        codecCtx->release_buffer = VideoDecoder::our_release_buffer;
    }   

    // If reach here, there is a video or audio or both streams is got.
    mpFormatCtx = pFormatCtx;
    miVideoStream = video_index;
    miAudioStream = audio_index;
    return 0;
}

int WindDecoder::Clear () {
    if (mpFormatCtx) {
        mbQuit = true;
        mVideoQueue.flush ();
        mAudioQueue.flush ();
        DEBUG ("begin wait thread!");
        ThreadWait ();
        DEBUG ("end wait thread!");
        if (miVideoStream >= 0) {
            WARN ("todo:clear video decoder!");
        }
        if (miAudioStream >= 0) {
            WARN ("todo:clear audio decoder!");
        }
        av_close_input_file (mpFormatCtx);
        mpFormatCtx = NULL;
        miVideoStream = -1;
        miAudioStream = -1;
        mVideoQueue.init ();
        mAudioQueue.init ();
        mbQuit = false;
    }
    return 0;
}

void WindDecoder::ThreadEntry () {
    AVFormatContext *pFormatCtx = mpFormatCtx;
    AVPacket pkt1, *packet = &pkt1;

    if (pFormatCtx == NULL) {
        ERROR ("pFormatCtx == NULL!");
        return;
    }

    // main decode loop

    for(;;) {
        if(mbQuit) {
            break;
        }

        if(av_read_frame(pFormatCtx, packet) < 0) {
            INFO ("av_read_frame fail, reach end of file or an error occur!");
            break;
        }

        // Is this a packet from the video stream?
        if(packet->stream_index == miVideoStream) {
            mVideoQueue.put (packet, packet->size);
			//DEBUG ("put video packet, pts: %lld, size:%d", packet->pts, packet->size);
        } else if(packet->stream_index == miAudioStream) {
            mAudioQueue.put (packet, packet->size);
			//DEBUG ("put audio packet, pts: %lld, size:%d", packet->pts, packet->size);
        } else {
            av_free_packet(packet);
        }
    }

    DEBUG ("end! insert null packet!");
    mVideoQueue.put ((AVPacket*)&PacketQueue::mNullPacket, 0);
    mAudioQueue.put ((AVPacket*)&PacketQueue::mNullPacket, 0);
    INFO ("end of demux!");
    return;
}

AVPacket WindDecoder::GetVideoPacket () {
    return mVideoQueue.get ();
}

AVPacket WindDecoder::GetAudioPacket () {
    return mAudioQueue.get ();
}

AVFormatContext* WindDecoder::GetFormatContext () {
    return mpFormatCtx;
}
int WindDecoder::GetVideoStreamIndex () {
    return miVideoStream;
}
int WindDecoder::GetAudioStreamIndex () {
    return miAudioStream;
}

int WindDecoder::AvSeekFrame (double pos) {
    if (mpFormatCtx == NULL) {
        ERROR ("mpFormatCtx == NULL");
        return -1;
    }
    int64_t seek_target = pos * AV_TIME_BASE;
    int stream_index = -1;
    int seek_flags = 0;//rel < 0 ? AVSEEK_FLAG_BACKWARD : 0;

    if (miVideoStream >= 0) stream_index = miVideoStream;
    else if (miAudioStream >= 0) stream_index = miAudioStream;

    if(stream_index >= 0){
        seek_target = av_rescale_q (seek_target, AV_TIME_BASE_Q, mpFormatCtx->streams[stream_index]->time_base);
    }
    if(av_seek_frame (mpFormatCtx, stream_index, seek_target, seek_flags) < 0) {
        ERROR("%s: error while seeking", mpFormatCtx->filename);
        return -1;
    }
    return 0;
}

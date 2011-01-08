#include "android_audio.h"
#include "util_log.h"
#define LOG_TAG "AndroidAudioOutput"

using namespace android;

// TODO: Find real cause of Audio/Video delay in PV framework and remove this workaround
/* static */ const uint32_t AndroidAudioOutput::kAudioVideoDelayMs = 0;
/* static */ int AndroidAudioOutput::mMinBufferCount = 4;
/* static */ bool AndroidAudioOutput::mIsOnEmulator = false;

static uint64_t lastWriteTime;

AndroidAudioOutput::AndroidAudioOutput () {
    mTrack = NULL;
    mStreamType = AudioSystem::MUSIC;
    mLeftVolume = 1.0;
    mRightVolume = 1.0;
    mLatency = 0;
    mMsecsPerFrame = 0;
    mNumFramesWritten = 0;
    mCallback = NULL;
    mCallbackCookie = NULL;
}
AndroidAudioOutput::~AndroidAudioOutput () {
}

int AndroidAudioOutput::Reset () {
    return 0;
}

int      AndroidAudioOutput::open (uint32_t sampleRate, int channelCount, int format, int bufferCount,
                                   AudioCallback cb, void *cookie) {
    mCallback = cb;
    mCallbackCookie = cookie;

    DEBUG ("open(%u, %d, %d, %d)", sampleRate, channelCount, format, bufferCount);
    if (mTrack) close();

    int afSampleRate;
    int afFrameCount;
    int frameCount;

    if (AudioSystem::getOutputFrameCount (&afFrameCount, mStreamType) != NO_ERROR) {
        ERROR ("AudioSystem::getOutputFrameCount fail!");
        return NO_INIT;
    }
    if (AudioSystem::getOutputSamplingRate (&afSampleRate, mStreamType) != NO_ERROR) {
        ERROR ("AudioSystem::getOutputSamplingRate fail!");
        return NO_INIT;
    }

    frameCount = (sampleRate*afFrameCount*bufferCount)/afSampleRate;

    AudioTrack *t;
    if (mCallback != NULL) {
        t = new AudioTrack(
                mStreamType,
                sampleRate,
                format,
                (channelCount == 2) ? AudioSystem::CHANNEL_OUT_STEREO : AudioSystem::CHANNEL_OUT_MONO,
                frameCount,
                0 /* flags */,
                CallbackWrapper,
                this);
    } else {
        t = new AudioTrack(
                mStreamType,
                sampleRate,
                format,
                (channelCount == 2) ? AudioSystem::CHANNEL_OUT_STEREO : AudioSystem::CHANNEL_OUT_MONO,
                frameCount);
    }

    if ((t == NULL) || (t->initCheck() != NO_ERROR)) {
        ERROR ("Unable to create audio track");
        delete t;
        return NO_INIT;
    }

    DEBUG ("setVolume: mLeftVolume=%f, mRightVolume=%f", mLeftVolume, mRightVolume);
    t->setVolume(mLeftVolume, mRightVolume);
    mMsecsPerFrame = 1.e3 / (float) sampleRate;
    mLatency = t->latency() + kAudioVideoDelayMs;
    mTrack = t;
    return NO_ERROR;
}
void     AndroidAudioOutput::start () {
    DEBUG ("start");
    if (mTrack) {
        mTrack->setVolume(mLeftVolume, mRightVolume);
        mTrack->start();
        mTrack->getPosition(&mNumFramesWritten);
    }
}
void     AndroidAudioOutput::stop () {
    LOGV("stop");
    if (mTrack) mTrack->stop();
    //lastWriteTime = 0;
}
void     AndroidAudioOutput::flush () {
    LOGV("flush");
    if (mTrack) mTrack->flush();
}
void     AndroidAudioOutput::pause () {
    LOGV("pause");
    if (mTrack) mTrack->pause();
    lastWriteTime = 0;
}
void     AndroidAudioOutput::close () {
    LOGV("close");
    delete mTrack;
    mTrack = NULL;
}
int      AndroidAudioOutput::write (const void* buffer, size_t size) {
    if (mCallback != NULL) {
        ERROR ("fatal error! Don't call write if supplying a callback!");
        return -1;
    }

    //DEBUG ("write(%p, %u)", buffer, size);
    if (mTrack) {
#if 0
        // Only make visualization buffers if anyone recently requested visualization data
        uint64_t now = uptimeMillis();
        if (lastReadTime + TOTALBUFTIMEMSEC >= now) {
            // Based on the current play counter, the number of frames written and
            // the current real time we can calculate the approximate real start
            // time of the buffer we're about to write.
            uint32_t pos;
            mTrack->getPosition(&pos);

            // we're writing ahead by this many frames:
            int ahead = mNumFramesWritten - pos;
            //LOGI("@@@ written: %d, playpos: %d, latency: %d", mNumFramesWritten, pos, mTrack->latency());
            // which is this many milliseconds, assuming 44100 Hz:
            ahead /= 44;

            makeVizBuffers((const char*)buffer, size, now + ahead + mTrack->latency());
            //lastWriteTime = now;
        }
#endif
        ssize_t ret = mTrack->write(buffer, size);
        mNumFramesWritten += ret / 4; // assume 16 bit stereo
        return ret;
    }
    return NO_INIT;
}
void     AndroidAudioOutput::setVolume (float left, float right) {
    LOGV("setVolume(%f, %f)", left, right);
    mLeftVolume = left;
    mRightVolume = right;
    if (mTrack) {
        mTrack->setVolume(left, right);
    }
}
    //int     dump (int fd, const Vector<String16>& args) const;

ssize_t  AndroidAudioOutput::bufferSize () const {
    if (mTrack == 0) return NO_INIT;
    return mTrack->frameCount() * frameSize();
}
ssize_t  AndroidAudioOutput::frameCount () const {
    if (mTrack == 0) return NO_INIT;
    return mTrack->frameCount();
}
ssize_t  AndroidAudioOutput::channelCount () const {
    if (mTrack == 0) return NO_INIT;
    return mTrack->channelCount();
}
ssize_t  AndroidAudioOutput::frameSize () const {
    if (mTrack == 0) return NO_INIT;
    return mTrack->frameSize();
}
uint32_t AndroidAudioOutput::latency () const {
    return mLatency;
}
float    AndroidAudioOutput::msecsPerFrame () const {
    return mMsecsPerFrame;
}

void AndroidAudioOutput::CallbackWrapper(int event, void *cookie, void *info) {
    if (event != AudioTrack::EVENT_MORE_DATA) {
        return;
    }

    AndroidAudioOutput *me = (AndroidAudioOutput *)cookie;
    AudioTrack::Buffer *buffer = (AudioTrack::Buffer *)info;

    (*me->mCallback)(me, buffer->raw, buffer->size, me->mCallbackCookie);
}

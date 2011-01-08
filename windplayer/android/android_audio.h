#ifndef __ANDROID_AUDIO_H__
#define __ANDROID_AUDIO_H__

#include <stdint.h>
#include <sys/types.h>

#include <ui/SurfaceComposerClient.h>

#include <media/AudioTrack.h>

class AndroidAudioOutput {
public:
    typedef void (*AudioCallback)(AndroidAudioOutput *audioSink, void *buffer, size_t size, void *cookie);

public:
    AndroidAudioOutput ();
    ~AndroidAudioOutput ();
    int Reset ();

    int      open (uint32_t sampleRate, int channelCount, int format, int bufferCount,
                   AudioCallback cb = NULL, void *cookie = NULL);
    void     start ();
    void     stop ();
    void     flush ();
    void     pause ();
    void     close ();
    int      write (const void* buffer, size_t size);
    void     setAudioStreamType (int streamType) { mStreamType = streamType; }
    void     setVolume (float left, float right);
    //int     dump (int fd, const Vector<String16>& args) const;

    ssize_t  bufferSize () const;
    ssize_t  frameCount () const;
    ssize_t  channelCount () const;
    ssize_t  frameSize () const;
    uint32_t latency () const;
    float    msecsPerFrame () const;

private:
    static void CallbackWrapper(int event, void *me, void *info);

    android::AudioTrack*   mTrack;
    AudioCallback mCallback;
    void *        mCallbackCookie;
    int           mStreamType;
    float         mLeftVolume;
    float         mRightVolume;
    float         mMsecsPerFrame;
    uint32_t      mLatency;

    // TODO: Find real cause of Audio/Video delay in PV framework and remove this workaround
    static const uint32_t   kAudioVideoDelayMs;
    static bool             mIsOnEmulator;
    static int              mMinBufferCount;  // 12 for emulator; otherwise 4

public: // visualization hack support
    uint32_t      mNumFramesWritten;
};

#endif

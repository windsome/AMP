#define LOG_TAG "WindPlayer-JNI"
//#include "utils/Log.h"

#include <stdio.h>
#include <assert.h>
#include <limits.h>
#include <unistd.h>
#include <fcntl.h>
#include "jni.h"
#include "JNIHelp.h"
#include "android_runtime/AndroidRuntime.h"
#include "android_util_Binder.h"
#include "android_windplayer.h"
#include "util_log.h"
using namespace android;

// ----------------------------------------------------------------------------
struct fields_t {
    jfieldID    context;
    jfieldID    surface;
    /* actually in android.view.Surface XXX */
    jfieldID    surface_native;
};
static fields_t fields;
static Mutex sLock;
// ----------------------------------------------------------------------------

static Surface* get_surface(JNIEnv* env, jobject clazz) {
    return (Surface*)env->GetIntField(clazz, fields.surface_native);
}

static AndroidWindPlayer* getWindPlayer(JNIEnv* env, jobject thiz) {
    Mutex::Autolock l(sLock);
    AndroidWindPlayer* const p = (AndroidWindPlayer*)env->GetIntField(thiz, fields.context);
    return p;
}

static AndroidWindPlayer* setWindPlayer(JNIEnv* env, jobject thiz, AndroidWindPlayer* player) {
    Mutex::Autolock l(sLock);
    AndroidWindPlayer* old = (AndroidWindPlayer*)env->GetIntField(thiz, fields.context);
    env->SetIntField(thiz, fields.context, (int)player);
    return old;
}

static int setVideoSurface(AndroidWindPlayer* mp, JNIEnv *env, jobject thiz) {
    jobject surface = env->GetObjectField(thiz, fields.surface);
    if (surface != NULL) {
        const sp<Surface> native_surface = get_surface(env, surface);
        DEBUG ("prepare: surface=%p (id=%d)", native_surface.get(), native_surface->ID());
        return mp->_setVideoSurface(native_surface);
    } else {
        ERROR ("surface == NULL!");
        return mp->_setVideoSurface(NULL);
    }
}

// ----------------------------------------------------------------------------
static int android_media_WindPlayer_release(JNIEnv *env, jobject thiz);
// ----------------------------------------------------------------------------

// This function gets some field IDs, which in turn causes class initialization.
// It is called from a static block in WindPlayer, which won't run until the
// first time an instance of this class is used.
static void android_media_WindPlayer_native_init(JNIEnv *env) {
    jclass clazz;

    clazz = env->FindClass("com/agou/media/WindPlayer");
    if (clazz == NULL) {
        jniThrowException(env, "java/lang/RuntimeException", "Can't find com/agou/media/WindPlayer");
        return;
    }

    fields.context = env->GetFieldID(clazz, "mNativeContext", "I");
    if (fields.context == NULL) {
        jniThrowException(env, "java/lang/RuntimeException", "Can't find WindPlayer.mNativeContext");
        return;
    }

    fields.surface = env->GetFieldID(clazz, "mSurface", "Landroid/view/Surface;");
    if (fields.surface == NULL) {
        jniThrowException(env, "java/lang/RuntimeException", "Can't find WindPlayer.mSurface");
        return;
    }

    jclass surface = env->FindClass("android/view/Surface");
    if (surface == NULL) {
        jniThrowException(env, "java/lang/RuntimeException", "Can't find android/view/Surface");
        return;
    }

    fields.surface_native = env->GetFieldID(surface, "mSurface", "I");
    if (fields.surface_native == NULL) {
        jniThrowException(env, "java/lang/RuntimeException", "Can't find Surface.mSurface");
        return;
    }
}

static void android_media_WindPlayer_native_setup(JNIEnv *env, jobject thiz, jobject weak_this) {
    LOGV("native_setup");
    AndroidWindPlayer* mp = new AndroidWindPlayer();
    if (mp == NULL) {
        jniThrowException(env, "java/lang/RuntimeException", "Out of memory");
        return;
    }

    // Stow our new C++ WindPlayer in an opaque field in the Java object.
    setWindPlayer(env, thiz, mp);
}

static void android_media_WindPlayer_native_finalize(JNIEnv *env, jobject thiz) {
    LOGV("native_finalize");
    android_media_WindPlayer_release(env, thiz);
}

static int android_media_WindPlayer_setVideoSurface(JNIEnv *env, jobject thiz) {
    AndroidWindPlayer* mp = getWindPlayer(env, thiz);
    if (mp == NULL ) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return -1;
    }
    return setVideoSurface(mp, env, thiz);
}

static int android_media_WindPlayer_setDataSource(JNIEnv *env, jobject thiz, jstring path) {
    AndroidWindPlayer* mp = getWindPlayer(env, thiz);
    if (mp == NULL ) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return -1;
    }

    if (path == NULL) {
        jniThrowException(env, "java/lang/IllegalArgumentException", NULL);
        return -1;
    }

    const char *pathStr = env->GetStringUTFChars(path, NULL);
    if (pathStr == NULL) {  // Out of memory
        jniThrowException(env, "java/lang/RuntimeException", "Out of memory");
        return -1;
    }
    LOGV("setDataSource: path %s", pathStr);
    int opStatus = mp->_setDataSource(pathStr);

    // Make sure that local ref is released before a potential exception
    env->ReleaseStringUTFChars(path, pathStr);
	return opStatus;
}

static int android_media_WindPlayer_start(JNIEnv *env, jobject thiz) {
    LOGV("start");
    AndroidWindPlayer* mp = getWindPlayer(env, thiz);
    if (mp == NULL ) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return -1;
    }
    return mp->_start();
}

static int android_media_WindPlayer_stop(JNIEnv *env, jobject thiz) {
    LOGV("stop");
    AndroidWindPlayer* mp = getWindPlayer(env, thiz);
    if (mp == NULL ) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return -1;
    }
    return mp->_stop();
}

static int android_media_WindPlayer_pause(JNIEnv *env, jobject thiz) {
    LOGV("pause");
    AndroidWindPlayer* mp = getWindPlayer(env, thiz);
    if (mp == NULL ) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return -1;
    }
    return mp->_pause();
}

static int android_media_WindPlayer_seek(JNIEnv *env, jobject thiz, int msec) {
    AndroidWindPlayer* mp = getWindPlayer(env, thiz);
    if (mp == NULL ) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return -1;
    }
    double sec = msec / 1000.0;
    LOGV("seekTo: %d(msec), %f(sec)", msec, sec);
	return mp->_seek(sec);
}

static int android_media_WindPlayer_release(JNIEnv *env, jobject thiz) {
    LOGV("release");
    AndroidWindPlayer* mp = setWindPlayer(env, thiz, 0);
    if (mp != NULL) {
        // this prevents native callbacks after the object is released
        return mp->_release ();
    }
	return -1;
}

static int android_media_WindPlayer_reset(JNIEnv *env, jobject thiz) {
    LOGV("reset");
    AndroidWindPlayer* mp = getWindPlayer(env, thiz);
    if (mp == NULL ) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return -1;
    }
    return mp->_reset();
}

static int android_media_WindPlayer_setVolume(JNIEnv *env, jobject thiz, float leftVolume, float rightVolume) {
    LOGV("setVolume: left %f  right %f", leftVolume, rightVolume);
    AndroidWindPlayer* mp = getWindPlayer(env, thiz);
    if (mp == NULL ) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return -1;
    }
    return mp->_setVolume(leftVolume, rightVolume);
}

static int android_media_WindPlayer_getVideoWidth(JNIEnv *env, jobject thiz) {
    AndroidWindPlayer* mp = getWindPlayer(env, thiz);
    if (mp == NULL ) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return -1;
    }
    return mp->getVideoWidth();
}

static int android_media_WindPlayer_getVideoHeight(JNIEnv *env, jobject thiz) {
    AndroidWindPlayer* mp = getWindPlayer(env, thiz);
    if (mp == NULL ) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return -1;
    }
    return mp->getVideoHeight();
}

static jboolean android_media_WindPlayer_isPlaying(JNIEnv *env, jobject thiz) {
    AndroidWindPlayer* mp = getWindPlayer(env, thiz);
    if (mp == NULL ) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return false;
    }
    const jboolean is_playing = mp->isPlaying();

    LOGV("isPlaying: %d", is_playing);
    return is_playing;
}

static int android_media_WindPlayer_getCurrentPosition(JNIEnv *env, jobject thiz) {
    AndroidWindPlayer* mp = getWindPlayer(env, thiz);
    if (mp == NULL ) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return -1;
    }
    double sec = mp->getCurrentPosition();
    int msec = sec * 1000;
    LOGV("getCurrentPosition: %d(msec), %f(sec)", msec, sec);
    return msec;
}

static int android_media_WindPlayer_getDuration(JNIEnv *env, jobject thiz) {
    AndroidWindPlayer* mp = getWindPlayer(env, thiz);
    if (mp == NULL ) {
        jniThrowException(env, "java/lang/IllegalStateException", NULL);
        return -1;
    }
    double sec = mp->getDuration();
    int msec = sec * 1000;
    LOGV("getDuration: %d(msec), %f(sec)", msec, sec);
    return msec;
}

static JNINativeMethod gMethods[] = {
    {"native_init",         "()V",                              (void *)android_media_WindPlayer_native_init},
    {"native_setup",        "(Ljava/lang/Object;)V",            (void *)android_media_WindPlayer_native_setup},
    {"native_finalize",     "()V",                              (void *)android_media_WindPlayer_native_finalize},
    {"_setVideoSurface",    "()I",                              (void *)android_media_WindPlayer_setVideoSurface},
    {"_setDataSource",      "(Ljava/lang/String;)I",            (void *)android_media_WindPlayer_setDataSource},
    {"_start",              "()I",                              (void *)android_media_WindPlayer_start},
    {"_stop",               "()I",                              (void *)android_media_WindPlayer_stop},
    {"_pause",              "()I",                              (void *)android_media_WindPlayer_pause},
    {"_seek",               "(I)I",                             (void *)android_media_WindPlayer_seek},
    {"_release",            "()I",                              (void *)android_media_WindPlayer_release},
    {"_reset",              "()I",                              (void *)android_media_WindPlayer_reset},
    {"_setVolume",          "(FF)I",                            (void *)android_media_WindPlayer_setVolume},
    {"getVideoWidth",       "()I",                              (void *)android_media_WindPlayer_getVideoWidth},
    {"getVideoHeight",      "()I",                              (void *)android_media_WindPlayer_getVideoHeight},
    {"isPlaying",           "()Z",                              (void *)android_media_WindPlayer_isPlaying},
    {"getCurrentPosition",  "()I",                              (void *)android_media_WindPlayer_getCurrentPosition},
    {"getDuration",         "()I",                              (void *)android_media_WindPlayer_getDuration},
};

jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
    JNIEnv* env = NULL;
    jint result = -1;

	LOGD ("JNI_OnLoad in media/jni");
    if (vm->GetEnv ((void**) &env, JNI_VERSION_1_4) != JNI_OK) {
        LOGE ("ERROR: GetEnv failed\n");
        goto bail;
    }
    assert(env != NULL);

    if (AndroidRuntime::registerNativeMethods (env, "com/agou/media/WindPlayer", gMethods, NELEM(gMethods)) < 0) {
        LOGE("ERROR: WindPlayer native registration failed\n");
        goto bail;
    }

    /* success -- return valid version number */
    result = JNI_VERSION_1_4;

bail:
    return result;
}

// KTHXBYE

package com.agou.media;

import android.content.ContentResolver;
import android.content.Context;
import android.content.res.AssetFileDescriptor;
import android.net.Uri;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.os.Parcel;
import android.os.ParcelFileDescriptor;
import android.os.PowerManager;
import android.util.Log;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.graphics.Bitmap;
import android.media.AudioManager;

import java.io.FileDescriptor;
import java.io.IOException;
import java.util.Set;
import java.lang.ref.WeakReference;

public class WindPlayer
{
    static {
        System.loadLibrary("windplayer2_jni");
        native_init();
    }

    private static native final void native_init();
    private native final void native_setup(Object mediaplayer_this);
    private native final void native_finalize();

    private native int _setVideoSurface();
    private native int _setDataSource(String path);
    private native int _start();
    private native int _stop();
    private native int _pause();
    private native int _seek(int msec);
    private native int _release();
    private native int _reset();
    private native int _setVolume(float leftVolume, float rightVolume);

    public native int getVideoWidth();
    public native int getVideoHeight();
    public native boolean isPlaying();
    public native int getCurrentPosition();
    public native int getDuration();

    private final static String TAG = "WindPlayer";
    private int mNativeContext; // accessed by native methods
    private int mListenerContext; // accessed by native methods
    private Surface mSurface; // accessed by native methods
    private SurfaceHolder mSurfaceHolder;
    private PowerManager.WakeLock mWakeLock = null;
    private boolean mScreenOnWhilePlaying;
    private boolean mStayAwake;

    public WindPlayer() {

        /* Native setup requires a weak reference to our object.
         * It's easier to create it here than in C++.
         */
        native_setup(new WeakReference<WindPlayer>(this));
    }

    public void setDisplay(SurfaceHolder sh) {
        mSurfaceHolder = sh;
        if (sh != null) {
            mSurface = sh.getSurface();
        } else {
            mSurface = null;
        }
        int ret = _setVideoSurface();
        if (ret < 0) {
        	Log.d(TAG, "setVideoSurface fail!");
        }
        updateSurfaceScreenOn();
    }

    /**
     * Sets the data source as a content Uri.
     *
     * @param context the Context to use when resolving the Uri
     * @param uri the Content URI of the data you want to play
     * @throws IllegalStateException if it is called in an invalid state
     */
    public int setDataSource(Context context, Uri uri) {
        String scheme = uri.getScheme();
        if(scheme == null || scheme.equals("file")) {
            int ret = _setDataSource(uri.getPath());
            if (ret < 0) {
            	Log.d(TAG, "setDataSource fail!");
            }
            return ret;
        } else {
            Log.d(TAG, "unrecognized file" + uri.toString());
            return -1;
        }
    }

    /**
     * Starts or resumes playback. If playback had previously been paused,
     * playback will continue from where it was paused. If playback had
     * been stopped, or never started before, playback will start at the
     * beginning.
     *
     * @throws IllegalStateException if it is called in an invalid state
     */
    public  void start() throws IllegalStateException {
        stayAwake(true);
        _start();
    }

    /**
     * Stops playback after playback has been stopped or paused.
     *
     * @throws IllegalStateException if the internal player engine has not been
     * initialized.
     */
    public void stop() throws IllegalStateException {
        stayAwake(false);
        _stop();
    }

    /**
     * Pauses playback. Call start() to resume.
     *
     * @throws IllegalStateException if the internal player engine has not been
     * initialized.
     */
    public void pause() throws IllegalStateException {
        stayAwake(false);
        _pause();
    }

    /**
     * Seek playback. 
     *
     * @throws IllegalStateException if the internal player engine has not been
     * initialized.
     */
    public void seekTo(int msec) throws IllegalStateException {
        stayAwake(false);
        _seek(msec);
    }

    /**
     * Releases resources associated with this WindPlayer object.
     * It is considered good practice to call this method when you're
     * done using the WindPlayer.
     */
    public void release() {
        stayAwake(false);
        updateSurfaceScreenOn();
        _release();
    }

    /**
     * Resets the WindPlayer to its uninitialized state. After calling
     * this method, you will have to initialize it again by setting the
     * data source and calling prepare().
     */
    public void reset() {
        stayAwake(false);
        _reset();
    }

    public int setVolume(float leftVolume, float rightVolume) {
    	return _setVolume(leftVolume, rightVolume);
    }

    @Override
    protected void finalize() { native_finalize(); }

    /**
     * Set the low-level power management behavior for this WindPlayer.  This
     * can be used when the WindPlayer is not playing through a SurfaceHolder
     * set with {@link #setDisplay(SurfaceHolder)} and thus can use the
     * high-level {@link #setScreenOnWhilePlaying(boolean)} feature.
     *
     * <p>This function has the WindPlayer access the low-level power manager
     * service to control the device's power usage while playing is occurring.
     * The parameter is a combination of {@link android.os.PowerManager} wake flags.
     * Use of this method requires {@link android.Manifest.permission#WAKE_LOCK}
     * permission.
     * By default, no attempt is made to keep the device awake during playback.
     *
     * @param context the Context to use
     * @param mode    the power/wake mode to set
     * @see android.os.PowerManager
     */
    public void setWakeMode(Context context, int mode) {
        boolean washeld = false;
        if (mWakeLock != null) {
            if (mWakeLock.isHeld()) {
                washeld = true;
                mWakeLock.release();
            }
            mWakeLock = null;
        }

        PowerManager pm = (PowerManager)context.getSystemService(Context.POWER_SERVICE);
        mWakeLock = pm.newWakeLock(mode|PowerManager.ON_AFTER_RELEASE, WindPlayer.class.getName());
        mWakeLock.setReferenceCounted(false);
        if (washeld) {
            mWakeLock.acquire();
        }
    }

    /**
     * Control whether we should use the attached SurfaceHolder to keep the
     * screen on while video playback is occurring.  This is the preferred
     * method over {@link #setWakeMode} where possible, since it doesn't
     * require that the application have permission for low-level wake lock
     * access.
     *
     * @param screenOn Supply true to keep the screen on, false to allow it
     * to turn off.
     */
    public void setScreenOnWhilePlaying(boolean screenOn) {
        if (mScreenOnWhilePlaying != screenOn) {
            mScreenOnWhilePlaying = screenOn;
            updateSurfaceScreenOn();
        }
    }

    private void stayAwake(boolean awake) {
        if (mWakeLock != null) {
            if (awake && !mWakeLock.isHeld()) {
                mWakeLock.acquire();
            } else if (!awake && mWakeLock.isHeld()) {
                mWakeLock.release();
            }
        }
        mStayAwake = awake;
        updateSurfaceScreenOn();
    }

    private void updateSurfaceScreenOn() {
        if (mSurfaceHolder != null) {
            mSurfaceHolder.setKeepScreenOn(mScreenOnWhilePlaying && mStayAwake);
        }
    }

    /**
     * Convenience method to create a WindPlayer for a given Uri.
     * On success, {@link #prepare()} will already have been called and must not be called again.
     * <p>When done with the WindPlayer, you should call  {@link #release()},
     * to free the resources. If not released, too many WindPlayer instances will
     * result in an exception.</p>
     *
     * @param context the Context to use
     * @param uri the Uri from which to get the datasource
     * @param holder the SurfaceHolder to use for displaying the video
     * @return a WindPlayer object, or null if creation failed
     */
    public static WindPlayer create(Context context, Uri uri, SurfaceHolder holder) {

        try {
            WindPlayer mp = new WindPlayer();
            mp.setDataSource(context, uri);
            if (holder != null) {
                mp.setDisplay(holder);
            }
            return mp;
        } catch (Exception ex) {
            Log.d(TAG, "create failed:", ex);
        }
        return null;
    }

}

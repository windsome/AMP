package com.agou.videoplayer;

/*
 * Copyright (C) 2006 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

import android.content.Context;
import android.content.Intent;
import android.content.res.Resources;
import android.media.AudioManager;
//import android.media.MediaPlayer;
import android.net.Uri;
import android.util.AttributeSet;
import android.util.Log;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.view.ViewGroup.LayoutParams;
import android.widget.MediaController;
import android.widget.MediaController.MediaPlayerControl;
import com.agou.media.WindPlayer;

import java.io.IOException;
 
/**
 * Displays a video file.  The VideoView class
 * can load images from various sources (such as resources or content
 * providers), takes care of computing its measurement from the video so that
 * it can be used in any layout manager, and provides various display options
 * such as scaling and tinting.
 */
public class VideoView extends SurfaceView implements MediaPlayerControl {
    public interface MySizeChangeLinstener{
    	public void doMyThings();
    }
    
    private String TAG = "VideoView";
    
    private Context mContext;
    
    // settable by the client
    private Uri         mUri;
    private int         mDuration;

    // All the stuff we need for playing and showing a video
    private SurfaceHolder mSurfaceHolder = null;
    private WindPlayer  mWindPlayer = null;
    private int         mVideoWidth;
    private int         mVideoHeight;
    private int         mSurfaceWidth;
    private int         mSurfaceHeight;
    private MediaController mMediaController;
    private int         mCurrentBufferPercentage;
    //private int         mSeekWhenPrepared;

    public int getVideoWidth(){
    	return mVideoWidth;
    }
    
    public int getVideoHeight(){
    	return mVideoHeight;
    }
    
    public void setVideoScale(int width , int height){
    	LayoutParams lp = getLayoutParams();
    	lp.height = height;
		lp.width = width;
		setLayoutParams(lp);
    }
    
    public VideoView(Context context) {
        super(context);
        mContext = context;
        initVideoView();
    }

    public VideoView(Context context, AttributeSet attrs) {
        this(context, attrs, 0);
        mContext = context;
        initVideoView();
    }

    public VideoView(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);
        mContext = context;
        initVideoView();
    }
    
    @Override
    protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
        //Log.i("@@@@", "onMeasure");
        int width = getDefaultSize(mVideoWidth, widthMeasureSpec);
        int height = getDefaultSize(mVideoHeight, heightMeasureSpec);
        /*if (mVideoWidth > 0 && mVideoHeight > 0) {
            if ( mVideoWidth * height  > width * mVideoHeight ) {
                //Log.i("@@@", "image too tall, correcting");
                height = width * mVideoHeight / mVideoWidth;
            } else if ( mVideoWidth * height  < width * mVideoHeight ) {
                //Log.i("@@@", "image too wide, correcting");
                width = height * mVideoWidth / mVideoHeight;
            } else {
                //Log.i("@@@", "aspect ratio is correct: " +
                        //width+"/"+height+"="+
                        //mVideoWidth+"/"+mVideoHeight);
            }
        }*/
        //Log.i("@@@@@@@@@@", "setting size: " + width + 'x' + height);
        setMeasuredDimension(width,height);
    }

    public int resolveAdjustedSize(int desiredSize, int measureSpec) {
        int result = desiredSize;
        int specMode = MeasureSpec.getMode(measureSpec);
        int specSize =  MeasureSpec.getSize(measureSpec);

        switch (specMode) {
            case MeasureSpec.UNSPECIFIED:
                /* Parent says we can be as big as we want. Just don't be larger
                 * than max size imposed on ourselves.
                 */
                result = desiredSize;
                break;

            case MeasureSpec.AT_MOST:
                /* Parent says we can be as big as we want, up to specSize.
                 * Don't be larger than specSize, and don't be larger than
                 * the max size imposed on ourselves.
                 */
                result = Math.min(desiredSize, specSize);
                break;

            case MeasureSpec.EXACTLY:
                // No choice. Do what we are told.
                result = specSize;
                break;
        }
        return result;
}

    private void initVideoView() {
        mVideoWidth = 0;
        mVideoHeight = 0;
        getHolder().addCallback(mSHCallback);
        getHolder().setType(SurfaceHolder.SURFACE_TYPE_PUSH_BUFFERS);
        setFocusable(true);
        setFocusableInTouchMode(true);
        requestFocus();
    }

    public void setVideoPath(String path) {
    	Log.d(TAG, "setVideoPath: "+path);
        setVideoURI(Uri.parse(path));
    }

    public void setVideoURI(Uri uri) {
    	Log.d(TAG, "setVideoURI: "+uri);
        mUri = uri;
        openVideo();
        requestLayout();
        invalidate();
    }

    public void stopPlayback() {
        if (mWindPlayer != null) {
            mWindPlayer.stop();
            mWindPlayer.release();
            mWindPlayer = null;
        }
    }

    private void openVideo() {
    	Log.d(TAG, "openVideo");
        if (mUri == null || mSurfaceHolder == null) {
            // not ready for playback just yet, will try again later
            return;
        }
        // Tell the music playback service to pause
        // TODO: these constants need to be published somewhere in the framework.
        Intent i = new Intent("com.android.music.musicservicecommand");
        i.putExtra("command", "pause");
        mContext.sendBroadcast(i);

        if (mWindPlayer != null) {
            mWindPlayer.reset();
            //mWindPlayer.release();
            //mWindPlayer = null;
        } else {
            mWindPlayer = new WindPlayer();
        }
        try {
            Log.v(TAG, "reset duration to -1 in openVideo");
            mDuration = -1;
            mCurrentBufferPercentage = 0;
            Log.v(TAG, "setDisplay()");
            mWindPlayer.setDisplay(mSurfaceHolder);
            Log.v(TAG, "setDataSource()");
            mWindPlayer.setDataSource(mContext, mUri);
            mVideoWidth = mWindPlayer.getVideoWidth();
            mVideoHeight = mWindPlayer.getVideoHeight();
            Log.v(TAG, "mSurfaceHolder.setFixedSize("+mVideoWidth+", "+mVideoHeight+")");
            mSurfaceHolder.setFixedSize(mVideoWidth, mVideoHeight);
            //mWindPlayer.setAudioStreamType(AudioManager.STREAM_MUSIC);
            mWindPlayer.setScreenOnWhilePlaying(true);
            attachMediaController();
        } catch (Exception ex) {
            Log.w(TAG, "Unable to open content: " + mUri, ex);
            return;
        }
    }

    public void setMediaController(MediaController controller) {
        if (mMediaController != null) {
            mMediaController.hide();
        }
        mMediaController = controller;
        attachMediaController();
    }

    private void attachMediaController() {
        if (mWindPlayer != null && mMediaController != null) {
            mMediaController.setMediaPlayer(this);
            View anchorView = this.getParent() instanceof View ?
                    (View)this.getParent() : this;
            mMediaController.setAnchorView(anchorView);
            //mMediaController.setEnabled(mIsPrepared);
        }
    }

    SurfaceHolder.Callback mSHCallback = new SurfaceHolder.Callback()
    {
        public void surfaceChanged(SurfaceHolder holder, int format,
                                    int w, int h)
        {
        	Log.d(TAG, "surfaceChanged, format="+format+", w="+w+", h="+h);
            mSurfaceWidth = w;
            mSurfaceHeight = h;
            if (mWindPlayer != null && mVideoWidth == w && mVideoHeight == h) {
                //mWindPlayer.start();
                if (mMediaController != null) {
                    mMediaController.show();
                }
            }
        }

        public void surfaceCreated(SurfaceHolder holder)
        {
        	Log.d(TAG, "surfaceCreated, holder="+holder);
            mSurfaceHolder = holder;
            openVideo();
        }

        public void surfaceDestroyed(SurfaceHolder holder)
        {
            // after we return from this we can't use the surface any more
        	Log.d(TAG, "surfaceDestroyed, holder="+holder);
            mSurfaceHolder = null;
            if (mMediaController != null) mMediaController.hide();
            if (mWindPlayer != null) {
                mWindPlayer.reset();
                mWindPlayer.release();
                mWindPlayer = null;
            }
        }
    };

    @Override
    public boolean onTouchEvent(MotionEvent ev) {
        if (mWindPlayer != null && mMediaController != null) {
            toggleMediaControlsVisiblity();
        }
        return false;
    }

    @Override
    public boolean onTrackballEvent(MotionEvent ev) {
        if (mWindPlayer != null && mMediaController != null) {
            toggleMediaControlsVisiblity();
        }
        return false;
    }

    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event)
    {
        if (    keyCode != KeyEvent.KEYCODE_BACK &&
                keyCode != KeyEvent.KEYCODE_VOLUME_UP &&
                keyCode != KeyEvent.KEYCODE_VOLUME_DOWN &&
                keyCode != KeyEvent.KEYCODE_MENU &&
                keyCode != KeyEvent.KEYCODE_CALL &&
                keyCode != KeyEvent.KEYCODE_ENDCALL &&
                mWindPlayer != null &&
                mMediaController != null) {
            if (keyCode == KeyEvent.KEYCODE_HEADSETHOOK ||
                    keyCode == KeyEvent.KEYCODE_MEDIA_PLAY_PAUSE) {
                if (mWindPlayer.isPlaying()) {
                    pause();
                    mMediaController.show();
                } else {
                    start();
                    mMediaController.hide();
                }
                return true;
            } else if (keyCode == KeyEvent.KEYCODE_MEDIA_STOP
                    && mWindPlayer.isPlaying()) {
                pause();
                mMediaController.show();
            } else {
                toggleMediaControlsVisiblity();
            }
        }

        return super.onKeyDown(keyCode, event);
    }

    private void toggleMediaControlsVisiblity() {
    	Log.d(TAG, "toggleMediaControlsVisiblity");
        if (mMediaController.isShowing()) {
            mMediaController.hide();
        } else {
            mMediaController.show();
        }
    }

    public void start() {
    	Log.d(TAG, "start");
        if (mWindPlayer != null) {
        	mWindPlayer.start();
        } else {
        }
    }

    public void pause() {
    	Log.d(TAG, "pause");
        if (mWindPlayer != null) {
            if (mWindPlayer.isPlaying()) {
                mWindPlayer.pause();
            }
        }
    }

    public int getDuration() {
    	Log.d(TAG, "getDuration");
        if (mWindPlayer != null) {
            if (mDuration > 0) {
                return mDuration;
            }
            mDuration = mWindPlayer.getDuration();
            return mDuration;
        }
        mDuration = -1;
        return mDuration;
    }

    public int getCurrentPosition() {
    	//Log.v(TAG, "getCurrentPosition");
       if (mWindPlayer != null) {
            return mWindPlayer.getCurrentPosition();
        }
        return 0;
    }

    public void seekTo(int msec) {
    	Log.d(TAG, "seekTo");
        if (mWindPlayer != null) {
            mWindPlayer.seekTo(msec);
        }
    }

    public boolean isPlaying() {
    	//Log.v(TAG, "isPlaying");
        if (mWindPlayer != null) {
            return mWindPlayer.isPlaying();
        }
        return false;
    }

    public int getBufferPercentage() {
    	Log.d(TAG, "getBufferPercentage");
        if (mWindPlayer != null) {
            return mCurrentBufferPercentage;
        }
        return 0;
    }
    public boolean canPause() {
		// unimplement
        return false;
	}
    public boolean canSeekBackward() {
		// unimplement
        return false;
	}
    public boolean canSeekForward() {
		// unimplement
        return false;
	}

}

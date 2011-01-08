package com.agou.videoplayer;

import java.io.File;
import java.io.FileFilter;
import java.net.MalformedURLException;
import java.net.URISyntaxException;
import java.net.URL;
import java.util.LinkedList;
import org.apache.http.client.entity.UrlEncodedFormEntity;

import com.agou.videoplayer.SoundView.OnVolumeChangedListener;
import com.agou.videoplayer.VideoView.MySizeChangeLinstener;
import com.agou.videoplayer.R;

import android.R.integer;
import android.app.Activity;
import android.app.Dialog;
import android.content.Context;
import android.content.Intent;
import android.content.res.Configuration;
import android.database.Cursor;
import android.media.AudioManager;
import android.media.MediaPlayer;
import android.media.MediaPlayer.OnCompletionListener;
import android.media.MediaPlayer.OnPreparedListener;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.os.MessageQueue.IdleHandler;
import android.provider.MediaStore;
import android.util.Log;
import android.view.Display;
import android.view.Gravity;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;
import android.view.View.OnClickListener;
import android.view.View.OnLongClickListener;
import android.view.ViewGroup.LayoutParams;
import android.widget.AnalogClock;
import android.widget.Button;
import android.widget.ImageButton;
import android.widget.PopupWindow;
import android.widget.SeekBar;
import android.widget.TextView;
import android.widget.SeekBar.OnSeekBarChangeListener;
import android.view.GestureDetector;
import android.view.GestureDetector.SimpleOnGestureListener;

public class VideoPlayerActivity extends Activity {
	
	private final static String TAG = "VideoPlayerActivity";
	
	public static LinkedList<MovieInfo> playList = new LinkedList<MovieInfo>();
	public class MovieInfo{
		String displayName;  
		String path;
	}
	private Uri videoListUri = MediaStore.Video.Media.EXTERNAL_CONTENT_URI;
	private static int position ;
	private static boolean backFromAD = false;
	private int playedTime;
	
	private VideoView vv = null;
	private SeekBar seekBar = null;  
	private TextView durationTextView = null;
	private TextView playedTextView = null;
	private GestureDetector mGestureDetector = null;
	private AudioManager mAudioManager = null;  
	
	private int maxVolume = 0;
	private int currentVolume = 0;  
	
	private ImageButton bn1 = null;
	private ImageButton bn2 = null;
	private ImageButton bn3 = null;
	private ImageButton bn4 = null;
	private ImageButton bn5 = null;
	
	private View controlView = null;
	private PopupWindow controler = null;
	
	private SoundView mSoundView = null;
	private PopupWindow mSoundWindow = null;
	
	private View extralView = null;
	private PopupWindow extralWindow = null;
	
	private static int screenWidth = 0;
	private static int screenHeight = 0;
	private static int controlHeight = 0;  
	
	private final static int TIME = 6868;  
	
	private boolean isControllerShow = true;
	private boolean isPaused = false;
	private boolean isFullScreen = false;
	private boolean isSilent = false;
	private boolean isSoundShow = false;
	
    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) {
    	
        super.onCreate(savedInstanceState);  
        setContentView(R.layout.main);
        
        Log.d("OnCreate", getIntent().toString());
        
        Looper.myQueue().addIdleHandler(new IdleHandler() {

			public boolean queueIdle() {
				
				// TODO Auto-generated method stub
				if(controler != null && vv.isShown()){
					controler.showAtLocation(vv, Gravity.BOTTOM, 0, 0);
					//controler.update(screenWidth, controlHeight);
					controler.update(0, 0, screenWidth, controlHeight);
				}
				
				if(extralWindow != null && vv.isShown()){
					extralWindow.showAtLocation(vv,Gravity.TOP,0, 0);
					extralWindow.update(0, 25, screenWidth, 60);
				}
				
				//myHandler.sendEmptyMessageDelayed(HIDE_CONTROLER, TIME);
				return false;  
			}
        });
        
        
        controlView = getLayoutInflater().inflate(R.layout.controler, null);
        controler = new PopupWindow(controlView);
        durationTextView = (TextView) controlView.findViewById(R.id.duration);
        playedTextView = (TextView) controlView.findViewById(R.id.has_played);
        
        mSoundView = new SoundView(this);
        mSoundView.setOnVolumeChangeListener(new OnVolumeChangedListener(){

			public void setYourVolume(int index) {
				 
				cancelDelayHide();
				updateVolume(index);
				hideControllerDelay();
			}
        });
        
        mSoundWindow = new PopupWindow(mSoundView);
        
        extralView = getLayoutInflater().inflate(R.layout.extral, null);
        extralWindow = new PopupWindow(extralView);
        
        ImageButton backButton = (ImageButton) extralView.findViewById(R.id.back);
        ImageButton aboutButton = (ImageButton) extralView.findViewById(R.id.about);  
        
        position = -1;
        
        backButton.setOnClickListener(new OnClickListener(){

			public void onClick(View v) {
				// TODO Auto-generated method stub
				VideoPlayerActivity.this.finish();
			}
        	
        });
        aboutButton.setOnClickListener(new OnClickListener(){
        	
        	Dialog dialog;
        	OnClickListener mClickListener = new OnClickListener(){

				public void onClick(View v) {
					// TODO Auto-generated method stub
					Log.d("DIALOG", "DISMISS");
					dialog.dismiss();
					//vv.seekTo(msec);
					vv.start();
				}
			};
			public void onClick(View v) {
				// TODO Auto-generated method stub
				/*Intent intent = new Intent();
				intent.setClass(VideoPlayerActivity.this, VideoChooseActivity.class);
				VideoPlayerActivity.this.startActivityForResult(intent, 0);*/
				
				dialog = new Dialog(VideoPlayerActivity.this);
				dialog.getWindow().requestFeature(Window.FEATURE_NO_TITLE);
				View view = VideoPlayerActivity.this.getLayoutInflater().inflate(R.layout.about, null);
				dialog.setContentView(view);
				view.findViewById(R.id.cancel).setOnClickListener(mClickListener);
				vv.pause();
				dialog.show();
				
				cancelDelayHide();
			}
        	
        });
        
        bn1 = (ImageButton) controlView.findViewById(R.id.button1);
        bn2 = (ImageButton) controlView.findViewById(R.id.button2);
        bn3 = (ImageButton) controlView.findViewById(R.id.button3);
        bn4 = (ImageButton) controlView.findViewById(R.id.button4);
        bn5 = (ImageButton) controlView.findViewById(R.id.button5);
        
        vv = (VideoView) findViewById(R.id.vv);
 
        Uri uri = getIntent().getData();
        if(uri!=null){
        	if(vv.getVideoHeight()==0){
        		vv.setVideoURI(uri);
        	}
        	bn3.setImageResource(R.drawable.pause);
        }else{
        	bn3.setImageResource(R.drawable.play);
        }

        getVideoFile(playList, new File("/sdcard/"));
        
        Cursor cursor = getContentResolver().query(videoListUri, new String[]{"_display_name","_data"}, null, null, null);
        int n = cursor.getCount();
        cursor.moveToFirst();
        LinkedList<MovieInfo> playList2 = new LinkedList<MovieInfo>();
        for(int i = 0 ; i != n ; ++i){
        	MovieInfo mInfo = new MovieInfo();
        	mInfo.displayName = cursor.getString(cursor.getColumnIndex("_display_name"));
        	mInfo.path = cursor.getString(cursor.getColumnIndex("_data"));
        	playList2.add(mInfo);
        	cursor.moveToNext();
        }
        
        if(playList2.size() > playList.size()){
        	playList = playList2;
        }
        /*
        vv.setMySizeChangeLinstener(new MySizeChangeLinstener(){

			public void doMyThings() {
				// TODO Auto-generated method stub
				setVideoScale(SCREEN_DEFAULT);
			}
        	
        });*/
              
        bn1.setAlpha(0xBB);
        bn2.setAlpha(0xBB);  
        bn3.setAlpha(0xBB);
        bn4.setAlpha(0xBB);
        
        mAudioManager = (AudioManager) getSystemService(Context.AUDIO_SERVICE);
        maxVolume = mAudioManager.getStreamMaxVolume(AudioManager.STREAM_MUSIC);
        currentVolume = mAudioManager.getStreamVolume(AudioManager.STREAM_MUSIC);
        bn5.setAlpha(findAlphaFromSound());
        
        bn1.setOnClickListener(new OnClickListener(){

			public void onClick(View arg0) {
				// TODO Auto-generated method stub
				Intent intent = new Intent();
				intent.setClass(VideoPlayerActivity.this, VideoChooseActivity.class);
				VideoPlayerActivity.this.startActivityForResult(intent, 0);
				
				cancelDelayHide();
			}
        	
        });
        
        bn4.setOnClickListener(new OnClickListener(){

			public void onClick(View v) {
				// TODO Auto-generated method stub
				int n = playList.size();
				if(++position < n){
					vv.setVideoPath(playList.get(position).path);
					cancelDelayHide();
					hideControllerDelay();
				}else{
					VideoPlayerActivity.this.finish();
				}
			}
        	
        });
        
        bn3.setOnClickListener(new OnClickListener(){

			public void onClick(View v) {
				// TODO Auto-generated method stub
				cancelDelayHide();
				if(isPaused){
					vv.start();
					bn3.setImageResource(R.drawable.pause);
					hideControllerDelay();
				}else{
					vv.pause();
					bn3.setImageResource(R.drawable.play);
				}
				isPaused = !isPaused;
				
			}
        	
        });
        
        bn2.setOnClickListener(new OnClickListener(){

			public void onClick(View v) {
				// TODO Auto-generated method stub
				if(--position>=0){
					vv.setVideoPath(playList.get(position).path);
					cancelDelayHide();
					hideControllerDelay();
				}else{
					VideoPlayerActivity.this.finish();
				}
			}
        	
        });
        
        bn5.setOnClickListener(new OnClickListener(){

		public void onClick(View v) {
			// TODO Auto-generated method stub
			cancelDelayHide();
			if(isSoundShow){
				mSoundWindow.dismiss();
			}else{
				if(mSoundWindow.isShowing()){
					mSoundWindow.update(15,0,SoundView.MY_WIDTH,SoundView.MY_HEIGHT);
				}else{
					mSoundWindow.showAtLocation(vv, Gravity.RIGHT|Gravity.CENTER_VERTICAL, 15, 0);
					mSoundWindow.update(15,0,SoundView.MY_WIDTH,SoundView.MY_HEIGHT);
				}
			}
			isSoundShow = !isSoundShow;
			hideControllerDelay();
		}   
       });
        
        bn5.setOnLongClickListener(new OnLongClickListener(){

			public boolean onLongClick(View arg0) {
				// TODO Auto-generated method stub
				if(isSilent){
					bn5.setImageResource(R.drawable.soundenable);
				}else{
					bn5.setImageResource(R.drawable.sounddisable);
				}
				isSilent = !isSilent;
				updateVolume(currentVolume);
				cancelDelayHide();
				hideControllerDelay();
				return true;
			}
        	
        });
        
        seekBar = (SeekBar) controlView.findViewById(R.id.seekbar);
        seekBar.setOnSeekBarChangeListener(new OnSeekBarChangeListener(){

				public void onProgressChanged(SeekBar seekbar, int progress, boolean fromUser) {
					// TODO Auto-generated method stub
					
					if(fromUser){
						
						vv.seekTo(progress);
					}
					
				}
	
				public void onStartTrackingTouch(SeekBar arg0) {
					// TODO Auto-generated method stub
					myHandler.removeMessages(HIDE_CONTROLER);
				}
	
				public void onStopTrackingTouch(SeekBar seekBar) {
					// TODO Auto-generated method stub
					myHandler.sendEmptyMessageDelayed(HIDE_CONTROLER, TIME);
				}
        	});
        
        getScreenSize();
       
        mGestureDetector = new GestureDetector(new SimpleOnGestureListener(){

			@Override
			public boolean onDoubleTap(MotionEvent e) {
				// TODO Auto-generated method stub
				if(isFullScreen){
					setVideoScale(SCREEN_DEFAULT);
				}else{
					setVideoScale(SCREEN_FULL);
				}
				isFullScreen = !isFullScreen;
				Log.d(TAG, "onDoubleTap");
				
				if(isControllerShow){
					showController();
				}
				//return super.onDoubleTap(e);
				return true;
			}

			@Override
			public boolean onSingleTapConfirmed(MotionEvent e) {
				// TODO Auto-generated method stub
				if(!isControllerShow){
					showController();
					hideControllerDelay();
				}else {
					cancelDelayHide();
					hideController();
				}
				//return super.onSingleTapConfirmed(e);
				return true;
			}

			@Override
			public void onLongPress(MotionEvent e) {
				// TODO Auto-generated method stub
				if(isPaused){
					vv.start();
					bn3.setImageResource(R.drawable.pause);
					cancelDelayHide();
					hideControllerDelay();
				}else{
					vv.pause();
					bn3.setImageResource(R.drawable.play);
					cancelDelayHide();
					showController();
				}
				isPaused = !isPaused;
				//super.onLongPress(e);
			}	
        });
        /*
        vv.setOnCompletionListener(new OnCompletionListener(){

				public void onCompletion(MediaPlayer arg0) {
					// TODO Auto-generated method stub
					int n = playList.size();
					if(++position < n){
						vv.setVideoPath(playList.get(position).path);
					}else{
						VideoPlayerActivity.this.finish();
					}
				}
        	});*/
    }

    @Override
	protected void onActivityResult(int requestCode, int resultCode, Intent data) {
		// TODO Auto-generated method stub
    	if(requestCode==0&&resultCode==Activity.RESULT_OK){
    		int result = data.getIntExtra("CHOOSE", -1);
    		Log.d(TAG, "onActivityResult, result="+result);
    		if(result!=-1){
    			vv.setVideoPath(playList.get(result).path);
        		
                int i = vv.getDuration();
                seekBar.setMax(i);
                i/=1000;
                int minute = i/60;
                int hour = minute/60;
                int second = i%60;
                minute %= 60;
                durationTextView.setText(String.format("%02d:%02d:%02d", hour,minute,second));
    			
                myHandler.sendEmptyMessage(PROGRESS_CHANGED);
                
    			position = result;
    		}
    		
    		return ;
    	}
		super.onActivityResult(requestCode, resultCode, data);
	}

	private final static int PROGRESS_CHANGED = 0;
    private final static int HIDE_CONTROLER = 1;
    
    Handler myHandler = new Handler(){
    
    	String mPlayedTime = "";
		@Override
		public void handleMessage(Message msg) {
			// TODO Auto-generated method stub
			//Log.d(TAG, "handleMessage:" + msg);
			switch(msg.what){
			
				case PROGRESS_CHANGED:
					
					int i = vv.getCurrentPosition();
					seekBar.setProgress(i);
					if (vv.isPlaying()) {
						i/=1000;
						int minute = i/60;
						int hour = minute/60;
						int second = i%60;
						minute %= 60;
						mPlayedTime = String.format("%02d:%02d:%02d", hour,minute,second);
					}
					playedTextView.setText(mPlayedTime);
					sendEmptyMessageDelayed (PROGRESS_CHANGED, 800);
					break;
					
				case HIDE_CONTROLER:
					Log.d(TAG, "handleMessage:" + msg);
					hideController();
					break;
			}
			
			super.handleMessage(msg);
		}	
    };

	@Override
	public boolean onTouchEvent(MotionEvent event) {
		// TODO Auto-generated method stub
		Log.d(TAG, "onTouchEvent"+event);
		boolean result = mGestureDetector.onTouchEvent(event);
		
		if(!result){
			if(event.getAction()==MotionEvent.ACTION_UP){
				
				/*if(!isControllerShow){
					showController();
					hideControllerDelay();
				}else {
					cancelDelayHide();
					hideController();
				}*/
			}
			result = super.onTouchEvent(event);
		}
		
		return result;
	}

	@Override
	public void onConfigurationChanged(Configuration newConfig) {
		// TODO Auto-generated method stub
		Log.d(TAG, "onConfigurationChanged"+newConfig);
		getScreenSize();
		if(isControllerShow){
			
			cancelDelayHide();
			hideController();
			showController();
			hideControllerDelay();
		}
		
		super.onConfigurationChanged(newConfig);
	}

	@Override
	protected void onPause() {
		// TODO Auto-generated method stub
		Log.d(TAG, "onPause");
		playedTime = vv.getCurrentPosition();
		vv.pause();
		bn3.setImageResource(R.drawable.play);
		super.onPause();   
	}

	@Override
	protected void onResume() {
		// TODO Auto-generated method stub
		Log.d(TAG, "onResume");
		vv.seekTo(playedTime);
		vv.start();  
		if(vv.getVideoHeight()!=0){
			bn3.setImageResource(R.drawable.pause);
			hideControllerDelay();
		}
		
		super.onResume();
	}

	@Override
	protected void onDestroy() {
		// TODO Auto-generated method stub
		
		if(controler.isShowing()){
			controler.dismiss();
			extralWindow.dismiss();
		}
		if(mSoundWindow.isShowing()){
			mSoundWindow.dismiss();
		}
		
		myHandler.removeMessages(PROGRESS_CHANGED);
		myHandler.removeMessages(HIDE_CONTROLER);
		
		playList.clear();
		
		super.onDestroy();
	}     

	private void getScreenSize()
	{
		Display display = getWindowManager().getDefaultDisplay();
        screenHeight = display.getHeight();
        screenWidth = display.getWidth();
        controlHeight = screenHeight/4;
        Log.d(TAG, "getScreenSize: height="+screenHeight+", width="+screenWidth+", controlHeight="+controlHeight);
	}
	
	private void hideController(){
		Log.d(TAG, "hideController");
		if(controler.isShowing()){
			controler.update(0,0,0, 0);
			extralWindow.update(0,0,screenWidth,0);
			isControllerShow = false;
		}
		if(mSoundWindow.isShowing()){
			mSoundWindow.dismiss();
			isSoundShow = false;
		}
	}
	
	private void hideControllerDelay(){
		Log.d(TAG, "hideControllerDelay");
		myHandler.sendEmptyMessageDelayed(HIDE_CONTROLER, TIME);
	}
	
	private void showController(){
		Log.d(TAG, "showController");
		controler.update(0,0,screenWidth, controlHeight);
		if(isFullScreen){
			extralWindow.update(0,0,screenWidth, 60);
		}else{
			extralWindow.update(0,25,screenWidth, 60);
		}
		
		isControllerShow = true;
	}
	
	private void cancelDelayHide(){
		Log.d(TAG, "cancelDelayHide");
		myHandler.removeMessages(HIDE_CONTROLER);
	}

    private final static int SCREEN_FULL = 0;
    private final static int SCREEN_DEFAULT = 1;
    
    private void setVideoScale(int flag){
		Log.d(TAG, "setVideoScale");
    	
    	LayoutParams lp = vv.getLayoutParams();
    	
    	switch(flag){
    		case SCREEN_FULL:
    			
    			Log.d(TAG, "screenWidth: "+screenWidth+" screenHeight: "+screenHeight);
    			vv.setVideoScale(screenWidth, screenHeight);
    			getWindow().addFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN);
    			
    			break;
    			
    		case SCREEN_DEFAULT:
    			
    			int videoWidth = vv.getVideoWidth();
    			int videoHeight = vv.getVideoHeight();
    			int mWidth = screenWidth;
    			int mHeight = screenHeight - 25;
    			
    			if (videoWidth > 0 && videoHeight > 0) {
    	            if ( videoWidth * mHeight  > mWidth * videoHeight ) {
    	                //Log.i("@@@", "image too tall, correcting");
    	            	mHeight = mWidth * videoHeight / videoWidth;
    	            } else if ( videoWidth * mHeight  < mWidth * videoHeight ) {
    	                //Log.i("@@@", "image too wide, correcting");
    	            	mWidth = mHeight * videoWidth / videoHeight;
    	            } else {
    	                
    	            }
    	        }
    			
    			vv.setVideoScale(mWidth, mHeight);

    			getWindow().clearFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN);
    			
    			break;
    	}
    }

    private int findAlphaFromSound(){
		Log.d(TAG, "findAlphaFromSound");
    	if(mAudioManager!=null){
    		//int currentVolume = mAudioManager.getStreamVolume(AudioManager.STREAM_MUSIC);
    		int alpha = currentVolume * (0xCC-0x55) / maxVolume + 0x55;
    		return alpha;
    	}else{
    		return 0xCC;
    	}
    }

    private void updateVolume(int index){
		Log.d(TAG, "updateVolume");
    	if(mAudioManager!=null){
    		if(isSilent){
    			mAudioManager.setStreamVolume(AudioManager.STREAM_MUSIC, 0, 0);
    		}else{
    			mAudioManager.setStreamVolume(AudioManager.STREAM_MUSIC, index, 0);
    		}
    		currentVolume = index;
    		bn5.setAlpha(findAlphaFromSound());
    	}
    }

    private void getVideoFile(final LinkedList<MovieInfo> list,File file){
		Log.d(TAG, "getVideoFile: begin>>"+file);
    	
    	file.listFiles(new FileFilter(){

			public boolean accept(File file) {
				// TODO Auto-generated method stub
				final String suffix[] = {".mp4", ".3gp", ".flv", ".mpg",
						".rm", "rmvb", ".wmv", ".avi", ".ts", 
						".mkv", ".mka", ".mp3", ".wav"};
				String name = file.getName();
				int i = name.indexOf('.');
				if(i != -1){
					name = name.substring(i);
					for (int j = 0; j < suffix.length; j++) {
						if (name.equalsIgnoreCase(suffix[j])) {
							MovieInfo mi = new MovieInfo();
							mi.displayName = file.getName();
							mi.path = file.getAbsolutePath();
							list.add(mi);
							Log.d(TAG, "getVideoFile: add>>"+mi);
							return true;
						}
					}
				}else if(file.isDirectory()){
					getVideoFile(list, file);
				}
				return false;
			}
    	});
    }
}

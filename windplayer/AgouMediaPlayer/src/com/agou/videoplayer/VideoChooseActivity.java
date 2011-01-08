package com.agou.videoplayer;

import java.util.LinkedList;
import com.agou.videoplayer.VideoPlayerActivity.MovieInfo;
import com.agou.videoplayer.R;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.Window;
import android.view.View.OnClickListener;
import android.widget.AdapterView;
import android.widget.BaseAdapter;
import android.widget.ImageButton;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.AdapterView.OnItemClickListener;

public class VideoChooseActivity extends Activity{

	private final static String TAG = "VideoChooseActivity";
	private static int height , width;
	private LinkedList<MovieInfo> mLinkedList;
	private LayoutInflater mInflater;
	View root;
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		// TODO Auto-generated method stub
		//Log.d(TAG, "onCreate");
		super.onCreate(savedInstanceState);
		getWindow().requestFeature(Window.FEATURE_NO_TITLE);
		setContentView(R.layout.dialog);
		
		mLinkedList = VideoPlayerActivity.playList;
		
		mInflater = getLayoutInflater();
		ImageButton iButton = (ImageButton) findViewById(R.id.cancel);
		iButton.setOnClickListener(new OnClickListener(){

			public void onClick(View arg0) {
				// TODO Auto-generated method stub
				Log.d(TAG, "onClick");
				VideoChooseActivity.this.finish();
			}
			
		});
		
		ListView myListView = (ListView) findViewById(R.id.list);
		myListView.setAdapter(new BaseAdapter(){

			public int getCount() {
				// TODO Auto-generated method stub
				//Log.d(TAG, "getCount ="+mLinkedList.size());
				return mLinkedList.size();
			}

			public Object getItem(int arg0) {
				// TODO Auto-generated method stub
				Log.d(TAG, "getItem "+arg0);
				return arg0;
			}

			public long getItemId(int arg0) {
				// TODO Auto-generated method stub
				Log.d(TAG, "getItemId " + arg0);
				return arg0;
			}

			public View getView(int arg0, View convertView, ViewGroup arg2) {
				// TODO Auto-generated method stub
				Log.d(TAG, "getView");
				if(convertView==null){
					convertView = mInflater.inflate(R.layout.list, null);
				}
				TextView text = (TextView) convertView.findViewById(R.id.text);
				text.setText(mLinkedList.get(arg0).displayName);
				
				return convertView;   
			}
			
		});
		
		myListView.setOnItemClickListener(new OnItemClickListener(){

			public void onItemClick(AdapterView<?> arg0, View arg1, int arg2,
					long arg3) {
				// TODO Auto-generated method stub
				Log.d(TAG, "onItemClick, arg0="+arg0+", arg1="+arg1+", arg2="+arg2+", arg3="+arg3);
				Intent intent = new Intent();
				intent.putExtra("CHOOSE", arg2);
				VideoChooseActivity.this.setResult(Activity.RESULT_OK, intent);
				VideoChooseActivity.this.finish();
				Log.d(TAG, "intent:"+intent);
			}
		});
	}
}

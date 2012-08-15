package com.y.player;

import android.app.Activity;
import android.app.Fragment;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

public class TailFragment extends Fragment{
	private final String TAG = Consts.TAG + "-TailFragment";
	public void onAttach (Activity activity){
		super.onAttach(activity);
		Log.i(TAG,"TailFragment:onAttach IN");
	}
	
	public void onCreate (Bundle savedInstanceState){
		super.onCreate(savedInstanceState);
		Log.i(TAG,"TailFragment:onCreate IN");
	}
	
	public View onCreateView (LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState){
		super.onCreateView(inflater, container, savedInstanceState);
		Log.i(TAG,"TailFragment:onCreateView IN");
		return inflater.inflate(R.layout.tail_fragment, container);
	}
	
	public void onStart (){
		super.onStart();
		Log.i(TAG,"TailFragment:onStart IN");
		TextView tv = (TextView) getView().findViewById(R.id.selected_svr);
		if(tv != null){
			tv.setText(R.string.local_svr);
		}
	}
	
	public void setTailFragmentMsg(CharSequence msg){
	}
	
	public void onPause(){
		super.onPause();
		Log.i(TAG,"TailFragment:onPause IN");
	}
	
	public void onResume (){
		super.onResume();
		Log.i(TAG,"TailFragment:onResume IN");
	}
	
	public void  onStop(){
		super.onStop();
		Log.i(TAG,"TailFragment:onStop IN");
	}
	
	public void onDetach (){
		super.onDetach();
		Log.i(TAG,"TailFragment:onDetach IN");
	}
	
	public void onDestroyView (){
		super.onDestroyView();
		Log.i(TAG,"TailFragment:onDestroyView IN");
	}
	
	public void onDestroy (){
		super.onDestroy();
		Log.i(TAG,"TailFragment:onDestroy IN");
	}
}
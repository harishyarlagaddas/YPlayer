package com.y.player;

import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.Button;

public class MusicFragment extends MediaFragment{
    private Button gPlayingButton = null;
    private final String TAG = Consts.TAG+"-MusicFragment";
    
	public void onStart(){
	    Log.i(TAG, "onStart IN");
	    if(gViewAdapter == null){
	        gViewAdapter = new ViewAdapter(getActivity(),Consts.MUSIC,gServerID);
	    }
		super.onStart();
		
		if(null != gPlayingButton){
		    gPlayingButton.setOnClickListener(new OnClickListener(){
                public void onClick(View arg0) {
                    if(mActivityListener != null){
                        mActivityListener.onPlayButtonClick();
                    }
                }
		    });
		}
	}
	
	public View onCreateView (LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState){
	   isGridViewLayout = true;
	   if(isGridViewLayout){
	       View v = inflater.inflate(R.layout.gridview_now_playing, container,false);
           gPlayingButton = (Button)v.findViewById(R.id.playingbutton);
           return v;
	   }else{
    	   View v = inflater.inflate(R.layout.listview_now_playing, container,false);
    	   gPlayingButton = (Button)v.findViewById(R.id.playingbutton);
    	   return v;
	   }
	}
}
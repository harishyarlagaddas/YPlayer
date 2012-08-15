package com.y.player;

import android.content.Intent;
import android.content.pm.PackageManager;
import android.content.pm.ResolveInfo;
import android.net.Uri;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Toast;

import java.util.List;

public class VideosFragment extends MediaFragment{
    private final String TAG = Consts.TAG+"-VideosFragment";
    public View onCreateView (LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState){
        Log.i(TAG,"onCreateView IN");
        isGridViewLayout = true;
		return super.onCreateView(inflater, container, savedInstanceState);
	}
	
	public void onStart(){
	    Log.i(TAG,"onStart IN");
	    if(gViewAdapter == null){
	        gViewAdapter = new ViewAdapter(getActivity(),Consts.VIDEOS,gServerID);
	    }
		super.onStart();
	}
	
	public void playVideo(MediaItem mi){
	    Log.i(TAG, "playVideo IN");
	    if(null == mi || null == mi.Path){
	        Log.e(TAG, "Error!! Media Item or the Name is NULL! So can not play.");
	        return;
	    }
	    Uri uri = Uri.parse(mi.Path);
	    /* Now Broadcast the Intent to play the video */
        Intent toPlay = new Intent(Intent.ACTION_VIEW);
        toPlay.setDataAndType(uri, "video/*");
        if(isAnySupportedApp(toPlay)){
            startActivity(toPlay);
        }else{
            getActivity().runOnUiThread(new Runnable(){
                public void run() {
                    Toast.makeText(getActivity().getApplicationContext(), R.string.error_no_suitable_app, Toast.LENGTH_SHORT).show();
                }
            });
        }
	}
	
	private boolean isAnySupportedApp(Intent intent) {
        List<ResolveInfo> list = getActivity().getPackageManager().queryIntentActivities(intent, PackageManager.MATCH_DEFAULT_ONLY);
        return list.size() > 0;
    }
}
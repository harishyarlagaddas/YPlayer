package com.y.player;

import java.util.ArrayList;

import android.content.Intent;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;

public class ImagesFragment extends MediaFragment{
    private final String TAG = Consts.TAG+"-ImagesFragment";
	public View onCreateView (LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState){
		isGridViewLayout = true;
	    Log.i(TAG,"onCreateView IN");
		return super.onCreateView(inflater, container, savedInstanceState);
	}
	
	public void onStart(){
	    Log.i(TAG,"onStart IN");
	    if(gViewAdapter == null){
	        gViewAdapter = new ViewAdapter(getActivity(),Consts.PICTURES,gServerID);
	    }
		super.onStart();
	}
	
	public void displayImage(ArrayList<MediaItem> list, int index){
        Log.i(TAG, "displayImage IN");
        if(null == list){
            Log.e(TAG, "Error!! Media List is NULL! So can not proceed!.");
            return;
        }
        ArrayList<String> nameList = new ArrayList<String>();
        for(int i=0; i<list.size(); i++){
            nameList.add(list.get(i).Path);
        }
        if(nameList.size() > 0 && index >= 0 && index < nameList.size()){
            Intent toDisplay = new Intent(getActivity().getApplicationContext(),ImageViewer.class);
            toDisplay.putStringArrayListExtra(Consts.PICTURE_LIST, nameList);
            toDisplay.putExtra(Consts.PICTURE_LIST_INDEX,index);
            getActivity().startActivity(toDisplay);
        }
    }
}
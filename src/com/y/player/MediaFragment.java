package com.y.player;

import android.app.Activity;
import android.app.Fragment;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.AdapterView.OnItemLongClickListener;
import android.widget.GridView;
import android.widget.ListView;

import java.util.ArrayList;

public class MediaFragment extends Fragment{
	private final String TAG = Consts.TAG + "-MediaFragment";
	
	protected ViewAdapter gViewAdapter = null;
	protected ListView gListView = null;
	protected GridView gGridView = null;
	protected boolean isGridViewLayout = false;
	protected boolean isInSelectionMode = false;
	protected OnMediaFragmentListener mActivityListener = null;
	protected int gServerID = Consts.LOCAL_HOST;
	
	// Container Activity must implement this interface
    public interface OnMediaFragmentListener {
        public void onMediaItemSelected(int i);
        public void play(ArrayList<MediaItem> list, int position);
        public void onPlayButtonClick();
    }
    
	public MediaFragment(){
		
	}
	
	public void onAttach (Activity activity){
		super.onAttach(activity);
		Log.i(TAG,"onAttach IN");
		try {
            mActivityListener = (OnMediaFragmentListener) activity;
        } catch (ClassCastException e) {
            throw new ClassCastException(activity.toString() + " must implement OnMediaFragmentListener");
        }
	}
	
	public void onCreate (Bundle savedInstanceState){
		super.onCreate(savedInstanceState);
		Log.i(TAG,"onCreate IN");
	}
	
	public View onCreateView (LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState){
		//super.onCreateView(inflater, container, savedInstanceState);
		Log.i(TAG,"onCreateView IN");
		View v = null;
		if(isGridViewLayout){
			 v = inflater.inflate(R.layout.grid_view, container,false);
		}else{
			v = inflater.inflate(R.layout.list_view, container,false);
		}
		return v;
	}
	
	public void onStart (){
		super.onStart();
		Log.i(TAG,"onStart IN");
		
		if(isGridViewLayout && gViewAdapter != null){
			//gViewAdapter.setGridViewLayout();
			gGridView = (GridView)getView().findViewById(R.id.grid_view);
			gGridView.setAdapter(gViewAdapter);
			gGridView.setOnItemClickListener(new OnItemClickListener(){

				public void onItemClick(AdapterView<?> parent, View view,final int position, long id) {
					// TODO Auto-generated method stub
                    if(mActivityListener != null){
                        mActivityListener.onMediaItemSelected(position);
                    }
				}
				
			});
			gGridView.setOnItemLongClickListener(new OnItemLongClickListener(){
				public boolean onItemLongClick(AdapterView<?> parent, View view, int position, long id) {
					// TODO Auto-generated method stub
					Log.d(TAG,"MediaFragMent:onItemLongClick: Clicked at position: "+position);
					return true;
				}
				
			});
		}else{
			gViewAdapter.setListViewLayout();
			gListView = (ListView) getView().findViewById(R.id.list_view);
			gListView.setAdapter(gViewAdapter);
			
			gListView.setClickable(true);
			//gListView.setChoiceMode(ListView.CHOICE_MODE_MULTIPLE);
			
			gListView.setOnItemClickListener(new AdapterView.OnItemClickListener() {
                public void onItemClick(AdapterView<?> parent, View view, final int position, long id) {
                    if(mActivityListener != null){
                        mActivityListener.onMediaItemSelected(position);
                    }
                }
			});
			gListView.setOnItemLongClickListener(new OnItemLongClickListener(){
				public boolean onItemLongClick(AdapterView<?> parent, View view, int position, long id) {
					// TODO Auto-generated method stub
					Log.d(TAG,"onItemLongClick: Clicked at position: "+position);
					return true;
				}
				
			});
		}
	}
	
	public void onPause(){
		super.onPause();
		Log.i(TAG,"onPause IN");
	}
	
	public void onResume (){
		super.onResume();
		Log.i(TAG,"onResume IN");
	}
	
	public void  onStop(){
		super.onStop();
		Log.i(TAG,"onStop IN");
	}
	
	public void onDetach (){
		super.onDetach();
		Log.i(TAG,"onDetach IN");
	}
	
	public void onDestroyView (){
		super.onDestroyView();
		Log.i(TAG,"onDestroyView IN");
	}
	
	public void onDestroy (){
		super.onDestroy();
		Log.i(TAG,"onDestroy IN");
	}
	
	public void selectServer(int serverID){
	    if(serverID == gServerID){
	        return;
	    }
	    gServerID = serverID;
	    if(gViewAdapter != null){
            gViewAdapter.changeServer(gServerID);
        }
	}
	
	public int getServerID(){
	    return gServerID;
	}
	
	public void addObjects(ArrayList<MediaItem> mediaList){
	    if(gViewAdapter != null){
	        gViewAdapter.AddNewMediaItems(mediaList);
	    }
	}
	
	public void onItemClick(int position){
	    if(gViewAdapter != null){
	        if(gViewAdapter.isPlayableItem(position)){
	            ArrayList<MediaItem> list = gViewAdapter.getCurrentList();
	            if(mActivityListener != null){
                    mActivityListener.play(list,position);
                }
	        }else{
	            gViewAdapter.onItemClick(position);
	        }
	    }
	}
	
	public boolean backPressed(){
	    if(gViewAdapter != null){
	        return gViewAdapter.backPressed();
	    }
	    return true;
	}
	
	public void downloadFilesFromServer(){
		if(null != gViewAdapter){
			gViewAdapter.downloadFilesFromServer();
		}
	}
}
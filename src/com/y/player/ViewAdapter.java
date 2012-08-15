package com.y.player;

import android.app.Activity;
import android.app.ProgressDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.DialogInterface.OnKeyListener;
import android.os.Environment;
import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.ImageView;
import android.widget.ProgressBar;
import android.widget.TextView;
import android.widget.Toast;

import java.util.ArrayList;
import java.util.Stack;

class MediaItem{
    public String Name;
    public int Duration;
    public String Path;
    public String ThumbnailPath;
    public int ItemType;
    public int ItemID;
    public boolean IsLocalItem;
    
    MediaItem(String name, String path, String thumbnailPath, int itemType){
        Name = name;
        Path = path;
        ThumbnailPath = thumbnailPath;
        ItemType = itemType;
        ItemID = 0;
        IsLocalItem = true;
        Duration = 0;
    };
    
    MediaItem(String name, String path, String thumbnailPath, int itemType, boolean localItem){
        Name = name;
        Path = path;
        ThumbnailPath = thumbnailPath;
        ItemType = itemType;
        ItemID = 0;
        IsLocalItem = localItem;
        Duration = 0;
    };
    
    MediaItem(String name, String path, String thumbnailPath, int itemType, int itemId, boolean isLocalItem){
        Name = name;
        Path = path;
        ThumbnailPath = thumbnailPath;
        ItemType = itemType;
        ItemID = itemId;
        IsLocalItem = isLocalItem;
        Duration = 0;
    };
};

public class ViewAdapter extends BaseAdapter implements AdapterInterface{
    
    private final String TAG = Consts.TAG + "-ViewAdapter";
    private LayoutInflater gInflater=null;
    public ImageLoader gImageLoader; 
    private ContentProvider gProvider = null;
    Stack<Stack<MediaItem>> gBackLists = null;
    private Stack<MediaItem>  gCurrentList = null;
    private int gItemsType = -1;
    private boolean isGridView = false;
    private Activity gActivity = null;
    private ProgressDialog gPD = null;
    private int gServerID = Consts.LOCAL_HOST;
    private int gCurrentObjID = 0;
    private int gTotalCount = 0;
    private int gCurrentIndex = 0;
    private int gCurrentWindow = Consts.MAIN_WINDOW;
    
    public static class ListViewHolder{
        public TextView name_text;
        public TextView album_text;
        public TextView size_text;
        public ImageView imageView;
        public ProgressBar progressbar;
    }
    
    public static class GridViewHolder{
        public TextView name_text;
        public ImageView imageView;
    }
    
    public ViewAdapter(Activity a,int itemsType, int serverID) {
        Log.d(TAG,"ViewAdapter:ViewAdapter: Creating the ViewAdapter for ItemsType = " +itemsType);
        gActivity = a;
        gServerID = serverID;
        gInflater = (LayoutInflater)gActivity.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        gProvider = new ContentProvider(gActivity.getApplicationContext(),this);
        gImageLoader=new ImageLoader(gActivity.getApplicationContext());
        gItemsType = itemsType;
        gCurrentList = new Stack<MediaItem>();
        gBackLists = new Stack<Stack<MediaItem>>();
        gPD = ProgressDialog.show(gActivity,gActivity.getString(R.string.app_name) , gActivity.getString(R.string.progress_msg),true);
        gPD.setOnKeyListener(new OnKeyListener(){
            public boolean onKey(DialogInterface arg0, int arg1, KeyEvent arg2) {
                if (arg1 == KeyEvent.KEYCODE_BACK || arg1 == KeyEvent.KEYCODE_HOME) {
                    Log.d(TAG, "Back/Home Key Pressed. Hence cancelling Progress Dialog");
                    if(gPD != null){
                        gPD.dismiss();
                        getBackList();
                    }
                }
                return true;
            }
        });
        showProgressDialog();
        new Thread(new Runnable(){
            public void run() {
                LoadContents();
            }
        }).start();
    }

    public int getCount() {
        return gCurrentList.size();
    }

    public Object getItem(int position) {
        return gCurrentList.get(position);
    }

    public long getItemId(int position) {
    	Log.d(TAG,"ViewAdapter: getItemID: "+position);
        return position;
    }
    
    public ArrayList<MediaItem> getCurrentList(){
        ArrayList<MediaItem> list = new ArrayList<MediaItem>();
        for(int i=0; i<gCurrentList.size(); i++){
            list.add(gCurrentList.get(i));
        }
        return list;
    }
    
    public View getView(int position, View currentView, ViewGroup parent) {
    	View v=currentView;
    	String name = gCurrentList.get(position).Name;
        String thumb = gCurrentList.get(position).ThumbnailPath;
        int itemType = gCurrentList.get(position).ItemType;
        boolean isLocalItem = gCurrentList.get(position).IsLocalItem;
    	if(isGridView){
	        GridViewHolder holder;
	        if(currentView==null){
	        	holder=new GridViewHolder();
	        	v = gInflater.inflate(R.layout.view_adapter_grid_view2, null);
	            holder.imageView=(ImageView)v.findViewById(R.id.imageview);
	            v.setTag(holder);
	        }else{
	            holder=(GridViewHolder)v.getTag();
	        }
	        gImageLoader.DisplayImage(thumb,itemType,isLocalItem, gActivity, holder.imageView,null);
    	}else{
	        ListViewHolder holder;
            //Log.d(TAG,"ViewAdapter:getView: setting Name Text as " +name +" and Album as "+album +" and size as " +size +" and Duration as "+duration);
            
	        if(currentView==null){
	            holder=new ListViewHolder();
	            v = gInflater.inflate(R.layout.view_adapter_list_view1, null);
                holder.name_text=(TextView)v.findViewById(R.id.name_text);
                holder.imageView=(ImageView)v.findViewById(R.id.imageview);
                holder.progressbar = (ProgressBar)v.findViewById(R.id.progress);
                v.setTag(holder);
	        }else{
	            holder=(ListViewHolder)v.getTag();
	        }
	        
	        if(name != null){
	        	holder.name_text.setText(name);
	        }else{
	        	Log.e(TAG,"ViewAdapter:getView name field is null");
	        }
	        gImageLoader.DisplayImage(thumb,itemType,isLocalItem, gActivity, holder.imageView,holder.progressbar);
    	}
        return v;
    }
    
    public void setGridViewLayout(){
    	isGridView = true;
    }
    
    public void setListViewLayout(){
    	isGridView = false;
    }
    
    public void showProgressDialog(){
        Log.i(TAG,"showProgressDialog IN");
        //if(!gPD.isShowing()){
            gActivity.runOnUiThread(new Runnable(){
                public void run() {
                    Log.d(TAG,"showProgressDialog showing the progressbar");
                    gPD.show();
                }
            });
        //}
    }

    public void dismissProgressDialog(){
        if(gPD != null && gPD.isShowing()){
            Log.d(TAG,"dismissProgressDialog dismissing the progressbar");
            gPD.hide();
        }
    }
    
    public boolean backPressed(){
        if(gCurrentWindow == Consts.MAIN_WINDOW){
            return true;
        }else{
            getBackList();
            return false;
        }
    }
    
    public void clearCurrentList(){
        if(gCurrentList == null || gCurrentList.size() <= 0){
            return;
        }
        Stack<MediaItem> tempList = new Stack<MediaItem>();
        for(int i=0; i<gCurrentList.size(); i++){
            MediaItem mi = gCurrentList.get(i);
            tempList.push(mi);
        }
        if(tempList.size() > 0){
            gBackLists.add(tempList);
        }
        
        gActivity.runOnUiThread(new Runnable(){
            public void run() {
                Log.d(TAG,"clearCurrentList IN Size: "+gCurrentList.size());
                gCurrentList.clear();
                notifyDataSetChanged();
                //showProgressDialog();
            }
        });
    }
    
    public void getBackList(){
        gActivity.runOnUiThread(new Runnable(){
            public void run() {
                gCurrentList.clear();
                if(gBackLists != null && gBackLists.size() > 0){
                    gCurrentList = gBackLists.pop();
                }
                
                if(gBackLists.size() <= 0){
                    gCurrentWindow = Consts.MAIN_WINDOW;
                }
                
                notifyDataSetChanged();
                //showProgressDialog();
            }
        });
    }
    
    public void resetItemLists(){
        if(gCurrentList != null){
            gCurrentList.clear();
        }
        if(gBackLists != null){
            gBackLists.clear();
        }
    }
    
    public void LoadContents(){
        clearCurrentList();
        if(Consts.LOCAL_HOST == gServerID){
            gProvider.getItems(gItemsType, gServerID, 0, 0, 0);
        }else{
            /* Need to Load the contents from DLNA Servers. */
            gTotalCount = gProvider.getNumberOfItems(gServerID, gCurrentObjID);
            gCurrentIndex = 0;
            getContentsFromServer(gCurrentObjID);
        }
    }
    
    public void getContentsFromServer(final int objID){        
        int count = 0;
        if(gTotalCount > 0){
            if(gTotalCount > Consts.GET_NUMBER_OF_ELEMENTS_COUNT){
                count = Consts.GET_NUMBER_OF_ELEMENTS_COUNT;
            }else{
                count = gTotalCount;
            }
        }
        if(count > 0){
            gProvider.getItems(gItemsType, gServerID, objID, gCurrentIndex, count);
            gCurrentIndex += count;
            gTotalCount -= count;
        }
    }
    
    public void changeServer(int serverID){
        Log.d(TAG,"Changing the serverID to  "+serverID+" for ItemsType: "+gItemsType);
        gServerID = serverID;
        gCurrentObjID = 0;
        gTotalCount = 0;
        gCurrentWindow = Consts.MAIN_WINDOW;
        showProgressDialog();
        resetItemLists();
        new Thread(new Runnable(){
            public void run() {
                LoadContents();     
            }
        }).start();
    }
    
    public void onItemClick(int position){
        if(gCurrentList != null){
            if(position > gCurrentList.size()){
                Log.d(TAG,"Selected Item id is greater than the size of the current list. Don't know what to do!!");
                return;
            }
            int objID = gCurrentList.get(position).ItemID;
            int itemType = gCurrentList.get(position).ItemType;
            if(Consts.FOLDERS == itemType){
                gCurrentWindow = Consts.SUB_WINDOW;
                gCurrentObjID = objID; 
                gTotalCount = 0;
                showProgressDialog();
                new Thread(new Runnable(){
                    public void run() {
                        LoadContents();
                    }
                }).start();
            }else{
                /* Need to handle how to play the corresponding contents. */
            }
        }
    }
    
    public boolean isPlayableItem(int position){
        if(gCurrentList != null){
            if(position > gCurrentList.size()){
                Log.d(TAG,"Selected Item id is greater than the size of the current list. Don't know what to do!!");
                return false;
            }
            int itemType = gCurrentList.get(position).ItemType;
            if(Consts.FOLDERS != itemType){
                return true;
            }
            return false;
        }
        return false;
    }
    
	public void AddNewMediaItems(final ArrayList<MediaItem> aMediaItemArray) {
		// TODO Auto-generated method stub
		gActivity.runOnUiThread(new Runnable(){
			public void run() {
			    if(null != aMediaItemArray){
    				for(int i=0; i < aMediaItemArray.size(); i++){
    					Log.i(TAG,"Adding the item with Name: "+aMediaItemArray.get(i).Name
    					        +" PATH: "+aMediaItemArray.get(i).Path
    					        +" Thumbnail Path: "+aMediaItemArray.get(i).ThumbnailPath
    					        +" ItemType: "+aMediaItemArray.get(i).ItemType
    					        +" ItemID: "+aMediaItemArray.get(i).ItemID
    					        +" isLocalItem: "+aMediaItemArray.get(i).IsLocalItem
    					        +" to the CurrentList of ItemsType: "+gItemsType);
    					gCurrentList.push(aMediaItemArray.get(i));
    				}
			    }
				dismissProgressDialog();
				notifyDataSetChanged();
				Log.d(TAG,"Calling notifyDataSetChanged with size: "+gCurrentList.size());
			}
		});
		if(Consts.LOCAL_HOST != gServerID){
    		new Thread(new Runnable(){
                public void run() {
                    getContentsFromServer(gCurrentObjID);
                }
    		}).start();
		}
	}
	
	public void downloadFilesFromServer(){
		new Thread(new Runnable(){
			public void run() {
				if(null != gCurrentList){
					String[] names = new String[gCurrentList.size()];
					String[] urls = new String[gCurrentList.size()];
					for(int i=0; i < gCurrentList.size(); i++){
						names[i] = gCurrentList.get(i).Name;
						urls[i] = gCurrentList.get(i).Path;
					}
					downloadFilesFromServer(names,urls);
				}
			}
		}).start();
	}
	
	public void downloadFilesFromServer(String names[], String urls[]){
		if(names.length == 0 || urls.length == 0){
			Log.d(TAG,"Incomming Names or URLS length is 0. So we can not proceed with download!!");
			return;
		}
		final String basePath = Environment.getExternalStorageDirectory() + Consts.DOWNLOADS_DIRECTORY;
		gActivity.runOnUiThread(new Runnable(){
            public void run() {
            	String mesg = gActivity.getApplicationContext().getString(R.string.downloading);
            	mesg = mesg.replace("%s", basePath);
                Toast.makeText(gActivity.getApplicationContext(), mesg, Toast.LENGTH_SHORT).show();     
            }
        });
		for(int i=0; i< names.length; i++){
			if(names[i].length() > 0 && urls[i].length() > 0){
				String name = basePath + names[i];
				Log.d(TAG, "Downloading the File: "+name);
				Utils.downloadFile(name, urls[i]);
			}
		}
	}
}
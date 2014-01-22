package com.y.player;

import android.content.ContentUris;
import android.content.Context;
import android.database.Cursor;
import android.net.Uri;
import android.provider.MediaStore;
import java.util.ArrayList;
import java.util.Stack;

public class ContentProvider{
	private Context gContext = null;
	private AdapterInterface gAdapterInterface = null;
	private static Stack<MediaItem> mAudioItemList = null;
	private static Stack<MediaItem> mVideoItemList = null;
	private static Stack<MediaItem> mPictureItemList = null;
	private static final String TAG = Consts.TAG +"-ContentProvider";
	private static JniInterface mJniInterface = null;
	
	public ContentProvider(Context ctx, AdapterInterface adapter){
		gContext = ctx;
		gAdapterInterface = adapter; 
		mJniInterface = MainActivity.getJNIInterface();
	}
	
	public void getItems(int contentType, int serverID, int itemID, int startingIndex, int limit){
	    Log.d(TAG,"getItems contentType = "+contentType+" serverID: "+serverID+" itemID: "+itemID+
	            " startingIndex: "+startingIndex +" limit: "+limit);
	    
	    if(serverID == Consts.LOCAL_HOST){
	        if(contentType == Consts.MUSIC){
	            getLocalMusicItems();
	        }else if(contentType == Consts.VIDEOS){
	            getLocalVideoItems();
	        }else if(contentType == Consts.PICTURES){
	            getLocalPictureItems();
	        }
	    }else{
	        /* DLNA Servers. Handle them appropriately. */
	        if(contentType == Consts.MUSIC){
	            getRemoteMusicItems(serverID,itemID,startingIndex,limit);
            }else if(contentType == Consts.VIDEOS){
                getRemoteVideoItems(serverID,itemID,startingIndex,limit);
            }else if(contentType == Consts.PICTURES){
                getRemotePictureItems(serverID,itemID,startingIndex,limit);
            }
	    }
	}
	
	public void getLocalMusicItems(){
	    new Thread(new Runnable(){
            public void run() {
                ArrayList<MediaItem> audioList = new ArrayList<MediaItem>();
                if(mAudioItemList != null){
                    Log.d(TAG,"ContentProvider:GetAllSongs already have the fetched list. Hence returning");
                    for(int i=0; i< mAudioItemList.size(); i++){
                        MediaItem m = mAudioItemList.get(i);
                        audioList.add(m);
                    }
                    gAdapterInterface.AddNewMediaItems(audioList);
                    return;
                }
                mAudioItemList = new Stack<MediaItem>();
                Cursor c1 = null;
                String[] cursor_cols = {
                        MediaStore.Audio.Media._ID,
                        MediaStore.Audio.Media.ALBUM_ID,
                        MediaStore.Audio.Media.ALBUM,
                        MediaStore.Audio.Media.TITLE,
                        MediaStore.Audio.Media.DATA,
                        MediaStore.Audio.Media.DURATION,
                        MediaStore.Audio.Media.SIZE,
                };
                c1 = gContext.getContentResolver().query(MediaStore.Audio.Media.EXTERNAL_CONTENT_URI, cursor_cols, null, null,
                                                                    MediaStore.Audio.Media.TITLE + " ASC");  
                if(c1 != null && c1.moveToFirst()){
                    int albumIdIndex = c1.getColumnIndex(android.provider.MediaStore.Audio.Media.ALBUM_ID);
                    int ci = c1.getColumnIndex(MediaStore.Audio.Media._ID);
                    int ci1 = c1.getColumnIndex(MediaStore.Audio.Media.TITLE);
                    int ci2 = c1.getColumnIndex(MediaStore.Audio.Media.DATA);
                    do{
                        /* First prepare the MediaItem and add to the list. So that when we want to play this particular 
                         * item we can get the details from this MediaItem. 
                         * */
                        /* Prepare albumart URI for this id */
                        Uri sArtworkUri = Uri.parse("content://media/external/audio/albumart");
                        Uri thumbUri = ContentUris.withAppendedId(sArtworkUri, c1.getLong(albumIdIndex));
                        String title = c1.getString(ci1);
                        String path = c1.getString(ci2);
                        MediaItem tempItem = new MediaItem(title,path,thumbUri.toString(),Consts.MUSIC);
                        tempItem.Path = path;
                        tempItem.ItemType = Consts.MUSIC;
                        tempItem.IsLocalItem = true;
                        //Log.d(TAG,"ContentProvider:GetAllSongs: Adding the item " +title.toString() + " with thumbnailPath = " +tempItem.ThumbnailPath +"  to ListAdapter");
                        mAudioItemList.add(tempItem);
                        audioList = new ArrayList<MediaItem>();
                        audioList.add(tempItem);
                        gAdapterInterface.AddNewMediaItems(audioList);
                    }while(c1.moveToNext());
                    c1.close();
                }else{
                    gAdapterInterface.AddNewMediaItems(null);
                }
            }
        }).start();
	}
	
	public void getRemoteMusicItems(int serverID, int itemID, int startingIndex,int limit){
	    Log.i(TAG,"getRemoteMusicItems serverID: "+serverID+" ItemID: "+itemID+" startingIndex: "+startingIndex+" limit: "+limit);
        if(mJniInterface != null){
            mJniInterface.GetAllMusicObjects(itemID, startingIndex, limit);
        }
        return;
    }
	
	public void getLocalVideoItems(){
        new Thread(new Runnable(){
            public void run() {
                ArrayList<MediaItem> videoList = new ArrayList<MediaItem>();
                if(mVideoItemList != null){
                    Log.d(TAG,"ContentProvider:GetAllVideos already have the fetched list. Hence returning");
                    for(int i=0; i< mVideoItemList.size(); i++){
                        MediaItem m = mVideoItemList.get(i);
                        videoList.add(m);
                    }
                    gAdapterInterface.AddNewMediaItems(videoList);
                    return;
                }
                mVideoItemList = new Stack<MediaItem>();
                Cursor c1 = null;
                String[] cursor_cols = {
                        MediaStore.Video.Media._ID,
                        MediaStore.Video.Media.TITLE,
                        MediaStore.Video.Media.DATA,
                        MediaStore.Video.Media.DURATION,
                        MediaStore.Video.Media.SIZE,
                        MediaStore.Video.Media.ALBUM,
                };
                c1 = gContext.getContentResolver().query(MediaStore.Video.Media.EXTERNAL_CONTENT_URI, cursor_cols, null, null,
                                                                    MediaStore.Video.Media.TITLE + " ASC");
                if(c1 != null && c1.moveToFirst()){
                    int ci = c1.getColumnIndex(MediaStore.Video.Media._ID);
                    int ci1 = c1.getColumnIndex(MediaStore.Video.Media.TITLE);
                    int ci2 = c1.getColumnIndex(MediaStore.Video.Media.DATA);
                    do{
                        /* First prepare the MediaItem and add to the list. So that when we want to play this particular 
                         * item we can get the details from this MediaItem. 
                         * */
                        String title = c1.getString(ci1);
                        String path = c1.getString(ci2);
                        String thumbnailPath = String.valueOf(c1.getInt(ci));
                        MediaItem tempItem = new MediaItem(title,path,thumbnailPath,Consts.VIDEOS);
                        tempItem.Path = path;
                        tempItem.ItemType = Consts.VIDEOS;
                        tempItem.IsLocalItem = true;
                        Log.d(TAG,"ContentProvider:GetAllVideos: Fetched the item " +title.toString() + " with thumbnailPath = " +tempItem.ThumbnailPath);
                        mVideoItemList.add(tempItem);
                        videoList = new ArrayList<MediaItem>();
                        videoList.add(tempItem);
                        gAdapterInterface.AddNewMediaItems(videoList);
                    }while(c1.moveToNext());
                    c1.close();
                }else{
                    gAdapterInterface.AddNewMediaItems(null);
                }
            }
        }).start();
    }
	    
	public void getRemoteVideoItems(int serverID,int itemID, int startingIndex,int limit){
	    Log.i(TAG,"getRemoteVideoItems serverID: "+serverID+" ItemID: "+itemID+" startingIndex: "+startingIndex+" limit: "+limit);
	    if(mJniInterface != null){
            mJniInterface.GetAllVideoObjects(itemID, startingIndex, limit);
        }
    }
	
	public void getLocalPictureItems(){
	    new Thread(new Runnable(){
            public void run() {
                ArrayList<MediaItem> pictureList = new ArrayList<MediaItem>();
                if(mPictureItemList != null){
                    Log.d(TAG,"ContentProvider:GetAllPictures already have the fetched list. Hence returning");
                    for(int i=0; i< mPictureItemList.size(); i++){
                        MediaItem m = mPictureItemList.get(i);
                        pictureList.add(m);
                    }
                    gAdapterInterface.AddNewMediaItems(pictureList);
                    return;
                }
                
                mPictureItemList = new Stack<MediaItem>();
                Cursor c1 = null;
                String[] cursor_cols = {
                        MediaStore.Images.Media._ID,
                        MediaStore.Images.Media.TITLE,
                        MediaStore.Images.Media.DATA,
                        MediaStore.Images.Media.SIZE,
                        MediaStore.Images.Media.DATE_MODIFIED,
                };
                c1 = gContext.getContentResolver().query(MediaStore.Images.Media.EXTERNAL_CONTENT_URI, cursor_cols, null, null,
                                                                    MediaStore.Images.Media.TITLE + " ASC");
                if(c1 != null && c1.moveToFirst()){
                    int ci = c1.getColumnIndex(MediaStore.Images.Media._ID);
                    int ci1 = c1.getColumnIndex(MediaStore.Images.Media.TITLE);
                    int ci2 = c1.getColumnIndex(MediaStore.Images.Media.DATA);
                    do{
                        /* First prepare the MediaItem and add to the list. So that when we want to play this particular 
                         * item we can get the details from this MediaItem. 
                         * */
                        String title = c1.getString(ci1);
                        String path = c1.getString(ci2);
                        String thumbnailPath = String.valueOf(c1.getInt(ci));
                        MediaItem tempItem = new MediaItem(title,path,thumbnailPath,Consts.PICTURES);
                        tempItem.Path = path;
                        tempItem.ItemType = Consts.PICTURES;
                        tempItem.IsLocalItem = true;
                        //Log.i(TAG,"ContentProvider:GetAllPictures: Fetched the item " +title.toString() + " with thumbnailPath = " +tempItem.ThumbnailPath);
                        mPictureItemList.add(tempItem);
                        pictureList = new ArrayList<MediaItem>();
                        pictureList.add(tempItem);
                        gAdapterInterface.AddNewMediaItems(pictureList);
                    }while(c1.moveToNext());
                    c1.close();
                }else{
                    gAdapterInterface.AddNewMediaItems(null);
                }
            }
        }).start();
	}
	
	public void getRemotePictureItems(int serverID,int itemID, int startingIndex,int limit){
	    Log.i(TAG,"getRemotePictureItems serverID: "+serverID+" ItemID: "+itemID+" startingIndex: "+startingIndex+" limit: "+limit);
	    if(mJniInterface != null){
            mJniInterface.GetAllPictureObjects(itemID, startingIndex, limit);
        }
    }
	
	public int getNumberOfItems(int serverID, int ObjectID){
	    int count = -2;
	    if(serverID == Consts.LOCAL_HOST){
	        count = -1;
	    }else{
	        if(mJniInterface != null){
	            count =  mJniInterface.GetNumOfChilds(ObjectID);
	        }
	    }
	    Log.d(TAG,"getNumberOfItems: serverID: "+serverID+" ObjectID: "+ObjectID+" Count: "+count);
	    return count;
	}
}

package com.y.player;

public class JniInterface {
    /* Define all the native functions here. */
    public native void initNativeLog();
    public native void closeNativeLog();
    public native void startControlPoint();
    public native void stopControlPoint();
    public native int selectServer(int serverId);
    public native void clearServerSelection();
    public native int selectRenderer(int rendererId);
    public native void getAllAlbums(int serverId, int limit);
    public native void getAllSongs(int serverId, int limit);
    public native void getAllPlaylists(int serverId, int limit);
    public native void getAllVideos(int serverId, int limit);
    public native void getAllPictures(int serverId, int limit);
    public native void getAllMusicObjects(int objectId, int startIndex,int limit);
    public native void getAllVideoObjects(int objectId, int startIndex, int limit);
    public native void getAllPictureObjects(int objectId, int startIndex,int limit);
    public native int getDuration(int objectId);
    public native int getNumOfChilds(int objectId);
    
    public void NewServerAdded(String s, int serverId){
        Log.d(TAG,"NewServerAdded serverName = " +s +" serverId "+serverId);
        gMainActivity.NewServerAdded(s, serverId);
    }
    
    public void NewRendererAdded(String s, int serverId){
        
    }
    
    public void NewServerRemoved(int serverId){
        Log.d(TAG,"NewServerRemoved serverId "+serverId);
        gMainActivity.ServerRemoved(serverId);
    }
    
    public void NewRendererRemoved(int serverId){
        Log.d(TAG,"NewRendererRemoved serverId "+serverId);
    }
    
    public void NotifyGetAllSongs(String[] songNames, int[] songIds, String[] songUris){
        Log.d(TAG,"NotifyGetAllSongs Number of SongNames =" + songNames.length);
    }
    
    public void NotifyGetAllVideos(String[] videoNames, int[] videoIds, String[] videoUris){
        Log.d(TAG,"NotifyGetAllVideos Number of VideoNames =" + videoNames.length);        
    }
    
    public void NotifyGetAllPictures(String[] pictureNames, int[] pictureIds, String[] pictureUris){
        Log.d(TAG,"NotifyGetAllPictures Number of PictureNames =" + pictureNames.length);        
    }
    
    public void NotifyGetMusicObjects(String[] objectNames, int[] objectIds, String[] objectUris,String[] thumbUris){
        Log.d(TAG,"NotifyGetMusicObjects Number of objectNames = " + objectNames.length);  
        gMainActivity.AddObjects(objectNames, objectIds, objectUris,thumbUris,Consts.MUSIC);
    }
    
    public void NotifyGetVideoObjects(String[] objectNames, int[] objectIds, String[] objectUris,String[] thumbUris){
        Log.d(TAG,"NotifyGetVideoObjects Number of objectNames = " + objectNames.length);  
        gMainActivity.AddObjects(objectNames, objectIds, objectUris,thumbUris,Consts.VIDEOS);
    }

    public void NotifyGetPictureObjects(String[] objectNames, int[] objectIds, String[] objectUris,String[] thumbUris){
        Log.d(TAG,"NotifyGetPictureObjects Number of objectNames = " + objectNames.length);
        gMainActivity.AddObjects(objectNames, objectIds, objectUris,thumbUris,Consts.PICTURES);
    }
    
    /* Define all Local variables here. */
    private static final String TAG = Consts.TAG + "-JNIInterface";
    private static MainActivity gMainActivity = null;
    private static boolean gControlPointStarted = false;
    private static int gServerSelected = -1;
    
    public void InitNativeLog(){
        Log.d(TAG,"InitNativeLog IN");
        initNativeLog();
    }
    
    public void CloseNativeLog(){
        Log.d(TAG,"InitNativeLog IN");
        closeNativeLog();
    }
    
    public void StartControlPoint(MainActivity player){
        Log.d(TAG,"StartControlPoint IN");
        gMainActivity = player;
        if(gControlPointStarted == false){
            startControlPoint();
            gControlPointStarted = true;
        }
    }
    
    public void StopControlPoint(){
        Log.d(TAG,"StopControlPoint IN");
        if(gControlPointStarted == true){
            stopControlPoint();
            gControlPointStarted = false;
        }
    }
    
    public int SelectServer(int serverId){
        Log.d(TAG,"SelectServer serverId =" + serverId);
        if(gServerSelected != serverId){
            return selectServer(serverId);
        }
        return 0;
    }
    
    public void ClearServerSelection(){
        Log.d(TAG,"ClearServerSelection IN");
        clearServerSelection();
    }
    
    public int SelectRenderer(int rendererId){
        Log.d(TAG,"SelectRenderer rendereId =" +rendererId);
        return selectRenderer(rendererId);
    }
    
    public void GetAllAlbums(int serverId, int limit){
        Log.d(TAG,"GetAllAlbums serverId ="+serverId +" limit = " +limit);
        getAllAlbums(serverId,limit);
    }
    
    public void GetAllSongs(int serverId, int limit){
        Log.d(TAG,"GetAllSongs serverId ="+serverId +" limit = " +limit);
        getAllSongs(serverId,limit);
    }
    
    public void GetAllPlaylists(int serverId, int limit){
        Log.d(TAG,"GetAllPlaylists serverId ="+serverId +" limit = " +limit);
        getAllPlaylists(serverId,limit);
    }
    
    public void GetAllVideos(int serverId, int limit){
        Log.d(TAG,"GetAllVideos serverId ="+serverId +" limit = " +limit);
        getAllVideos(serverId,limit);
    }
    
    public void GetAllPictures(int serverId, int limit){
        Log.d(TAG,"GetAllPictures serverId ="+serverId +" limit = " +limit);
        getAllPictures(serverId,limit);
    }
    
    public void GetAllMusicObjects(final int objectId,final int startIndex,final int limit){
        Log.d(TAG,"GetAllMusicObjects objectId ="+objectId +" startPoint = " +startIndex+" limt = "+limit);
        new Thread(new Runnable(){
            public void run() {
                getAllMusicObjects(objectId,startIndex,limit);
            }
            
        }).start();
    }
    
    public void GetAllVideoObjects(final int objectId, final int startIndex,final int limit){
        Log.d(TAG,"GetAllVideoObjects objectId ="+objectId +" startPoint = " +startIndex+" limt = "+limit);
        new Thread(new Runnable(){
            public void run() {
                getAllVideoObjects(objectId,startIndex,limit);
            }
            
        }).start();
    }
    
    public void GetAllPictureObjects(final int objectId,final int startIndex,final int limit){
        Log.d(TAG,"GetAllPictureObjects objectId ="+objectId +" startPoint = " +startIndex+" limt = "+limit);
        new Thread(new Runnable(){
            public void run() {
                getAllPictureObjects(objectId,startIndex,limit);
            }
        }).start();
    }
    
    public int GetDuration(int objectId){
        Log.d(TAG,"GetDuration objectId ="+objectId);
        return getDuration(objectId);
    }
    
    public int GetNumOfChilds(int objectId){
        Log.d(TAG,"GetNumOfChilds objectId ="+objectId);
        return getNumOfChilds(objectId);
    }
}

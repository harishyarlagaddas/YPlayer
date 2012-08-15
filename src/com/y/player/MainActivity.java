package com.y.player;

import java.util.ArrayList;

import android.app.ActionBar;
import android.app.ActionBar.Tab;
import android.app.ActionBar.TabListener;
import android.app.Activity;
import android.app.AlertDialog;
import android.app.FragmentTransaction;
import android.app.ProgressDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.DialogInterface.OnCancelListener;
import android.content.res.Configuration;
import android.os.Bundle;
import android.view.KeyEvent;
import android.view.Menu;
import android.view.MenuItem;
import android.widget.Toast;

import com.y.player.MediaFragment.OnMediaFragmentListener;

public class MainActivity extends Activity implements TabListener, OnMediaFragmentListener{
	
    static {
        System.loadLibrary("dlna-cp");
    }
    
	protected ActionBar gActionBar = null;
	public static Context gContext = null;
	protected static String TAG = Consts.TAG + "-MainActivity";
	
	/* Variables which holds the fragments of all the activities. */
	private MediaFragment gCurrentMediaFragment = null;
	private MusicFragment gMusicFragment = null;
	private ImagesFragment gImagesFragment = null;
	private VideosFragment gVideosFragment = null;
	private MusicPlayFragment gMusicPlayFragment = null;
	private ProgressDialog gPD = null;
	//private TailFragment gTailFragment = null;
	
	public static JniInterface gJNIInterface = null;
	private static ArrayList<Consts.ServerInfo> gServerList = null;
	private static int gSelectedServerID = Consts.LOCAL_HOST;
	private static String gSelectedServerName = null;
	
	private String MUSIC_LIST_FRAGMENT = null;
	private String VIDEO_LIST_FRAGMENT = null;
	private String PICTURE_LIST_FRAGMENT = null;
	
	private final int MUSIC_LIST_FRAGMENT_POSITION = 0;
	private final int VIDEO_LIST_FRAGMENT_POSITION = 1;
	private final int PICTURE_LIST_FRAGMENT_POSITION = 2;
	
	private int gCurrentTabPosition = 0;
	private boolean gShowMusicPlayFragment = false;
	/** Called when the activity is first created. */
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);
        
        gContext = this;
        MUSIC_LIST_FRAGMENT = getString(R.string.music_tab_name);
        VIDEO_LIST_FRAGMENT = getString(R.string.video_tab_name);
        PICTURE_LIST_FRAGMENT = getString(R.string.picture_tab_name);
        
        Log.InitLog();
        Log.i(TAG,"onCreate IN");
        /* Initializing the JNI Interface to interact with native DLNA-Control Point */
        gJNIInterface = new JniInterface();
        gJNIInterface.StartControlPoint(this);
        if(gServerList == null){
            gServerList = new ArrayList<Consts.ServerInfo>();
            Consts.ServerInfo sInfo = new Consts.ServerInfo(getString(R.string.local_svr),Consts.LOCAL_HOST);
            gServerList.add(sInfo);
        }
        gSelectedServerName = getString(R.string.local_svr);
        gSelectedServerID = Consts.LOCAL_HOST;
        setTitleBar(gSelectedServerName);
        gMusicPlayFragment = new MusicPlayFragment();
        try {        
            gActionBar = getActionBar();
            gActionBar.setNavigationMode(ActionBar.NAVIGATION_MODE_TABS);
            
            gActionBar.addTab(gActionBar.newTab().setText(MUSIC_LIST_FRAGMENT).setTabListener(this), false);
            gActionBar.addTab(gActionBar.newTab().setText(VIDEO_LIST_FRAGMENT).setTabListener(this), false);
            gActionBar.addTab(gActionBar.newTab().setText(PICTURE_LIST_FRAGMENT).setTabListener(this), false);
            
            gActionBar.selectTab(gActionBar.getTabAt(MUSIC_LIST_FRAGMENT_POSITION));
            
        } catch (Exception e) {
        	Log.e(TAG,"Exception while getting the ActionBar and setting the ActioBar Attributes");
            e.printStackTrace();
            finish();
        }
    }
    
    public boolean onCreateOptionsMenu(Menu menu) {
    	Log.i(TAG,"MainActivity onCreateOptionsMenu IN");
      getMenuInflater().inflate(R.menu.options_menu, menu);
      return super.onCreateOptionsMenu(menu);
    }
    
    public boolean onPrepareOptionsMenu (Menu menu){
    	hideUnnecessayOptions(menu);
    	MenuItem mi = null;
    	if(gCurrentTabPosition == MUSIC_LIST_FRAGMENT_POSITION){
    		mi = menu.findItem(R.id.menu_id_download_all_songs);
    		if(null != mi){
    			mi.setVisible(true);
    		}
    	}else if(gCurrentTabPosition == VIDEO_LIST_FRAGMENT_POSITION){
    		mi = menu.findItem(R.id.menu_id_download_all_videos);
    		if(null != mi){
    			mi.setVisible(true);
    		}
    	}else if(gCurrentTabPosition == PICTURE_LIST_FRAGMENT_POSITION){
    		mi = menu.findItem(R.id.menu_id_download_all_images);
    		if(null != mi){
    			mi.setVisible(true);
    		}
    	}
		return true;
    }
    
    public void hideUnnecessayOptions(Menu menu){
    	MenuItem mi = null;
    	mi = menu.findItem(R.id.menu_id_download_all_songs);
		if(null != mi){
			mi.setVisible(false);
		}
		mi = menu.findItem(R.id.menu_id_download_all_videos);
		if(null != mi){
			mi.setVisible(false);
		}
		mi = menu.findItem(R.id.menu_id_download_all_images);
		if(null != mi){
			mi.setVisible(false);
		}
    }
    
    protected void onSaveInstanceState(Bundle outState) {
        //No call for super(). Bug on API Level > 11.
    }
    
    public void onConfigurationChanged (Configuration newConfig){
        super.onConfigurationChanged(newConfig);
        if(gCurrentTabPosition == MUSIC_LIST_FRAGMENT_POSITION){
            if(gShowMusicPlayFragment){
                /*if(null == gMusicPlayFragment){
                    gMusicPlayFragment = new MusicPlayFragment();
                }
                FragmentTransaction ft = getFragmentManager().beginTransaction();
                ft.detach(gMusicPlayFragment);
                ft.attach(gMusicPlayFragment);
                ft.commit();*/
            }else{
                if(null == gMusicFragment){
                    gMusicFragment = new MusicFragment();
                }
                FragmentTransaction ft = getFragmentManager().beginTransaction();
                ft.detach(gMusicFragment);
                ft.attach(gMusicFragment);
                ft.commit();
            }
        }else if(gCurrentTabPosition == VIDEO_LIST_FRAGMENT_POSITION){
            if(null == gVideosFragment){
                gVideosFragment = new VideosFragment();
            }
            FragmentTransaction ft = getFragmentManager().beginTransaction();
            ft.detach(gVideosFragment);
            ft.attach(gVideosFragment);
            ft.commit();
        }else if(gCurrentTabPosition == PICTURE_LIST_FRAGMENT_POSITION){
            if(null == gImagesFragment){
                gImagesFragment = new ImagesFragment();
            }
            FragmentTransaction ft = getFragmentManager().beginTransaction();
            ft.detach(gImagesFragment);
            ft.attach(gImagesFragment);
            ft.commit();
        }
    }
    
	public boolean onOptionsItemSelected (MenuItem item)
	{
		switch(item.getItemId())
		{
			case R.id.menu_id_refresh:{
				Toast.makeText(this, getString(R.string.refresh_msg), Toast.LENGTH_SHORT).show();
				refreshNetworkServers();
				break;
			}case R.id.menu_id_select_svr:{
			    /* Need to handle server Change.. */
			    showServerSelection();
			    break;
			}case R.id.menu_id_exit:{
			    finish();
			    exitApplication();
			    break;
            }case R.id.menu_id_download_all_songs:
             case R.id.menu_id_download_all_videos:
             case R.id.menu_id_download_all_images:{
            	if(gCurrentMediaFragment != null && Consts.LOCAL_HOST != gSelectedServerID){
            		gCurrentMediaFragment.downloadFilesFromServer();
            	}
            	break;
            }default:{
            	break;
            }
		}
		return true;
	}
    
	public void onBackPressed (){
	    super.onBackPressed();
	    exitApplication();
	}
	
	public void onDestroy(){
	    super.onDestroy();
	    if(null != gPD){
	        gPD.dismiss();
	    }
	}
	
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        if(keyCode == KeyEvent.KEYCODE_BACK) {
            boolean exit = true;
            if(gCurrentMediaFragment != null){
                exit = gCurrentMediaFragment.backPressed();
            }else if(gShowMusicPlayFragment){
                changeToMusicListTab();
                if(null != gMusicPlayFragment){
                    gMusicPlayFragment.stopProgressBarUpdate();
                }
                gShowMusicPlayFragment = false;
                exit = false;
            }
            if(true == exit){
                showExitDialog();
            }
            return true;
        }
        return false;
    }
	
    public void refreshNetworkServers(){
        gSelectedServerID = Consts.LOCAL_HOST;
        gSelectedServerName = getString(R.string.local_svr);
        setTitleBar(gSelectedServerName);
        if(null != gCurrentMediaFragment){
            gCurrentMediaFragment.selectServer(gSelectedServerID);
        }
        
        if(null != gJNIInterface){
            gJNIInterface.StopControlPoint();
            gServerList.clear();
            Consts.ServerInfo sInfo = new Consts.ServerInfo(getString(R.string.local_svr),Consts.LOCAL_HOST);
            gServerList.add(sInfo);
            gJNIInterface.startControlPoint();
        }
    }
    
    public void showExitDialog(){
        Log.d(TAG,"dlnaMiscUtils:ShowExitDialog: Showing Exit dialog");
        new AlertDialog.Builder(gContext)
        .setMessage("Do You Want To Exit?")
        .setPositiveButton("Yes", new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface dialog, int which) {
                onBackPressed();
            }
        })
        .setNegativeButton("No", null)
        .show();
    }
 
    public void exitApplication(){
        /* Do the necessary things before killing the application process itself.. */
        android.os.Process.killProcess(android.os.Process.myPid());
    }
    
	public void sendCommandToFragment(int command){
	    /* For sending the commands to Fragments. */
    }
    
    public void hideCurrentFragment(FragmentTransaction ft){
        if(gCurrentTabPosition == MUSIC_LIST_FRAGMENT_POSITION){
        	if(null != gMusicFragment && !gShowMusicPlayFragment){
        		ft.hide(gMusicFragment);
        	}
        	if(null != gMusicPlayFragment && gShowMusicPlayFragment){
        		ft.hide(gMusicPlayFragment);
        	}
        }else if(gCurrentTabPosition == VIDEO_LIST_FRAGMENT_POSITION){
            if(null != gVideosFragment){
                ft.hide(gVideosFragment);
            }
        }else if(gCurrentTabPosition == PICTURE_LIST_FRAGMENT_POSITION){
            if(null != gImagesFragment){
                ft.hide(gImagesFragment);
            }
        }
    }

    public void showCurrentFragment(FragmentTransaction ft){
        if(gCurrentTabPosition == MUSIC_LIST_FRAGMENT_POSITION){
            if(gShowMusicPlayFragment){
                if(null == gMusicPlayFragment){
                    gMusicPlayFragment = new MusicPlayFragment();
                }
                if(gMusicPlayFragment.isAdded()){
                    if(gMusicPlayFragment.isHidden()){
                        ft.show(gMusicPlayFragment);
                        ft.detach(gMusicPlayFragment);
                        ft.attach(gMusicPlayFragment);
                    }
                }else{
                    ft.add(R.id.fragment_holder,gMusicPlayFragment,MUSIC_LIST_FRAGMENT);
                }
                gCurrentMediaFragment = null;
            }else{
                if(null == gMusicFragment){
                    gMusicFragment = new MusicFragment();
                }
                if(gMusicFragment.isAdded()){
                    if(gMusicFragment.isHidden()){
                        ft.show(gMusicFragment);
                        ft.detach(gMusicFragment);
                        ft.attach(gMusicFragment);
                    }
                }else{
                    ft.add(R.id.fragment_holder,gMusicFragment,MUSIC_LIST_FRAGMENT);
                }
                gCurrentMediaFragment = gMusicFragment;
            }
        }else if(gCurrentTabPosition == VIDEO_LIST_FRAGMENT_POSITION){
            if(null == gVideosFragment){
                gVideosFragment = new VideosFragment();
            }
            if(gVideosFragment.isAdded()){
                if(gVideosFragment.isHidden()){
                    ft.show(gVideosFragment);
                    ft.detach(gVideosFragment);
                    ft.attach(gVideosFragment);
                }
            }else{
                ft.add(R.id.fragment_holder,gVideosFragment,VIDEO_LIST_FRAGMENT);
            }
            gCurrentMediaFragment = gVideosFragment;
        }else if(gCurrentTabPosition == PICTURE_LIST_FRAGMENT_POSITION){
            if(null == gImagesFragment){
                gImagesFragment = new ImagesFragment();
            }
            if(gImagesFragment.isAdded()){
                if(gImagesFragment.isHidden()){
                    ft.show(gImagesFragment);
                    ft.detach(gImagesFragment);
                    ft.attach(gImagesFragment);
                }
            }else{
                ft.add(R.id.fragment_holder,gImagesFragment,PICTURE_LIST_FRAGMENT);
            }
            gCurrentMediaFragment = gImagesFragment;
        }
    }
    
	public void onTabReselected(Tab tab, FragmentTransaction ft) {
		// TODO Auto-generated method stub
	}

	public void onTabSelected(Tab tab, FragmentTransaction ft) {
		hideCurrentFragment(ft);
		gCurrentTabPosition = tab.getPosition();
		showCurrentFragment(ft);
		if(gCurrentMediaFragment != null){
		    if(gSelectedServerID != gCurrentMediaFragment.getServerID()){
		        gCurrentMediaFragment.selectServer(gSelectedServerID);
		    }
		}
	}

	public void onTabUnselected(Tab tab, FragmentTransaction ft) {
		// TODO Auto-generated method stub
	}

	public void onMediaItemSelected(int position) {
		// TODO Auto-generated method stub
	    if(gCurrentMediaFragment != null){
	        gCurrentMediaFragment.onItemClick(position);
	    }
	}
	
	public void onPlayButtonClick() {
	    if(null != gMusicPlayFragment && gMusicPlayFragment.isPlaying()){
    	    gShowMusicPlayFragment = true;
    	    showProgressDialog(getString(R.string.play_msg));
            new Thread(new Runnable(){
                public void run() {
                    changeToMusicPlayTab();
                    dismissProgressDialog();
                    if(null != gMusicPlayFragment){
                        gMusicPlayFragment.startProgressBarUpdate();
                    }
                }
            }).start();
	    }else{
	        runOnUiThread(new Runnable(){
                public void run() {
                    Toast.makeText(gContext,getString(R.string.play_list_err_msg), Toast.LENGTH_SHORT).show();
                }
	        });
	    }
    }

	public void showProgressDialog(final String msg){
        Log.i(TAG,"showProgressDialog IN");
        //if(!gPD.isShowing()){
            runOnUiThread(new Runnable(){
                public void run() {
                    Log.d(TAG,"showProgressDialog showing the progressbar");
                    if(null == gPD){
                        gPD = ProgressDialog.show(gContext,getString(R.string.app_name),msg,true);
                    }else{
                        gPD.setMessage(msg);
                        gPD.show();
                    }
                }
            });
        //}
    }

    public void dismissProgressDialog(){
        if(gPD != null && gPD.isShowing()){
            runOnUiThread(new Runnable(){
                public void run() {
                    Log.d(TAG,"dismissProgressDialog dismissing the progressbar");
                    gPD.hide();
                }
            });
        }
    }
    
	public void changeToMusicPlayTab(){
	    FragmentTransaction ft = getFragmentManager().beginTransaction();
	    gCurrentTabPosition = gActionBar.getSelectedTab().getPosition();
	    if(gCurrentTabPosition == MUSIC_LIST_FRAGMENT_POSITION){
	        //hideCurrentFragment(ft);
	        if(null != gMusicFragment && !gMusicFragment.isHidden()){
	            ft.hide(gMusicFragment);
	        }
	        if(null == gMusicPlayFragment){
                gMusicPlayFragment = new MusicPlayFragment();
            }
	        if(gMusicPlayFragment.isAdded()){
	            ft.show(gMusicPlayFragment);
	            //ft.detach(gMusicPlayFragment);
                //ft.attach(gMusicPlayFragment);
	        }else{
	            ft.add(R.id.fragment_holder,gMusicPlayFragment,MUSIC_LIST_FRAGMENT);
	        }
	        //ft.replace(R.id.fragment_holder, gMusicPlayFragment);
	        ft.commit();
	        gCurrentMediaFragment = null;
	    }
	}
	
	public void changeToMusicListTab(){
        FragmentTransaction ft = getFragmentManager().beginTransaction();
        gCurrentTabPosition = gActionBar.getSelectedTab().getPosition();
        if(gCurrentTabPosition == MUSIC_LIST_FRAGMENT_POSITION){
            //hideCurrentFragment(ft);
            if(null != gMusicPlayFragment && !gMusicPlayFragment.isHidden()){
                ft.hide(gMusicPlayFragment);
            }
            if(null == gMusicFragment){
                gMusicFragment = new MusicFragment();
            }
            if(gMusicFragment.isAdded()){
                ft.show(gMusicFragment);
                ft.detach(gMusicFragment);
                ft.attach(gMusicFragment);
            }else{
                ft.add(R.id.fragment_holder,gMusicFragment,MUSIC_LIST_FRAGMENT);
            }
            //ft.replace(R.id.fragment_holder, gMusicFragment);
            ft.commit();
            gCurrentMediaFragment = gMusicFragment;
            if(gSelectedServerID != gCurrentMediaFragment.getServerID()){
                gCurrentMediaFragment.selectServer(gSelectedServerID);
            }
        }
    }
	
	public void play(final ArrayList<MediaItem> list,final int position){
        if(null == list){
            Log.e(TAG, "Error!! incomming list is NULL. Don't know how to play!!");
            return;
        }
        MediaItem mi= list.get(position);
        if(null == mi){
            Log.e(TAG, "Error!! Media Item is NULL. Don't know how to play");
            return;
        }
        int itemType = mi.ItemType;
        if(Consts.MUSIC == itemType){
            if(null != gActionBar){
                gShowMusicPlayFragment = true;
                showProgressDialog(getString(R.string.play_msg));
                new Thread(new Runnable(){
                    public void run() {
                        changeToMusicPlayTab();
                        if(gMusicPlayFragment != null){
                            gMusicPlayFragment.playSongFromList(list, position);
                        }
                        dismissProgressDialog();
                    }
                }).start();
            }
        }else if(Consts.VIDEOS == itemType){
            /* Need to handle Video Contents Play. */
            if(null != gVideosFragment){
                gVideosFragment.playVideo(mi);
            }
        }else if(Consts.PICTURES == itemType){
            /* Need to Handle Picture Display. */
            if(null != gImagesFragment){
                gImagesFragment.displayImage(list, position);
            }
        }
    }
	
	public void setTitleBar(String serverName){
	    runOnUiThread(new Runnable(){
            public void run() {
                setTitle(getString(R.string.app_name) +"-"+gSelectedServerName.trim());   
            }
	    });
	}
	
	public void showServerSelection(){
	    if(gServerList == null){
	        Log.d(TAG,"No Servers found on the current network. Sorry!!");
	        return;
	    }
	    final CharSequence[] items = new CharSequence[gServerList.size()];
	    for(int i=0; i<gServerList.size(); i++){
	        items[i] = gServerList.get(i).serverName;
	    }
	    AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setTitle(getString(R.string.app_name)+" - " +getString(R.string.select_svr));
        
        builder.setItems(items, new DialogInterface.OnClickListener() {    
            public void onClick(DialogInterface dialog, int item) {
                Log.d(TAG,"Selected the server: "+items[item]);
                dialog.dismiss();
                selectServer(gServerList.get(item));
            }
        });
        
        builder.setCancelable(false);   
        builder.setOnCancelListener(new OnCancelListener() {
           public void onCancel(DialogInterface dialog) {
               dialog.dismiss();
               finish();
               Log.d(TAG,"Dialog Cancelled.");
           } 
        });
        builder.setOnKeyListener(new DialogInterface.OnKeyListener() {
            public boolean onKey(DialogInterface dialog, int keyCode, KeyEvent event) {            
              if (keyCode == KeyEvent.KEYCODE_BACK || keyCode == KeyEvent.KEYCODE_HOME) {
                  Log.d(TAG, "Back/Home Key Pressed. Hence cancelling the builder options");
                  dialog.dismiss();
                  return true;
              }
              return false;
            }
        });
        builder.show();
	}
	
	public void selectServer(final Consts.ServerInfo serverInfo){
	    if(gJNIInterface != null && gSelectedServerID != serverInfo.serverID){
	        showProgressDialog(getString(R.string.select_msg));
	        ImageLoader.clearFileCache();
	        new Thread(new Runnable(){
                public void run() {
                    if(Consts.LOCAL_HOST != serverInfo.serverID){
                        gJNIInterface.ClearServerSelection();
                        if(0 != gJNIInterface.SelectServer(serverInfo.serverID)){
                            Log.d(TAG,"Failed to select the server. Name: "+serverInfo.serverName+" ID: "+serverInfo.serverID);
                            String msg = getString(R.string.select_svr_err);
                            msg = msg.replace("%s", serverInfo.serverName);
                            Toast.makeText(getApplicationContext(), msg, Toast.LENGTH_SHORT).show();
                            dismissProgressDialog();
                            return;
                        }
                    }
                    gSelectedServerID = serverInfo.serverID;
                    gSelectedServerName = serverInfo.serverName;
                    setTitleBar(gSelectedServerName);
                    dismissProgressDialog();
                    if(gCurrentMediaFragment != null){
                        gCurrentMediaFragment.selectServer(gSelectedServerID);
                    }
                }
	        }).start();
	    }
	}
	
	public static JniInterface getJNIInterface(){
	    return gJNIInterface;
	}
	
	/* Functions called from JNI Interface */
	public void NewServerAdded(final String s, final int serverId){
	    Log.d(TAG,"NewServerAdded. Server Name: "+s +" serverID: "+serverId);
	    Consts.ServerInfo sInfo = new Consts.ServerInfo(s,serverId);
	    gServerList.add(sInfo);
	    runOnUiThread(new Runnable(){
            public void run() {
                String msg = getString(R.string.new_svr_added);
                msg = msg.replace("%s", s);
                Toast.makeText(getApplicationContext(), msg, Toast.LENGTH_SHORT).show();
            }
	    });
	}
	
	public void ServerRemoved(int serverID){
	    Consts.ServerInfo sInfo = null;
	    for(int i=0; i < gServerList.size(); i++){
	        sInfo = gServerList.get(i);
	        if(serverID == sInfo.serverID){
	            gServerList.remove(i);
	            if(serverID == gSelectedServerID){
	                gSelectedServerID = Consts.LOCAL_HOST;
	                gSelectedServerName = getString(R.string.local_svr);
	                if(gCurrentMediaFragment != null){
                        gCurrentMediaFragment.selectServer(gSelectedServerID);
                    }
	            }
	        }
	    }
	}
	
	public void AddObjects(final String[] objectNames, final int[] objectIds, final String[] objectUris,
	                        final String[] thumbUris,final int objectType){
	    
        Log.d(TAG,"AddObjects IN Number Of Objects: = " +objectNames.length);
        ArrayList<MediaItem> mediaItemArray = new ArrayList<MediaItem>();
        for(int i=0; i<objectNames.length; i++){
            int itemType = objectType; 
            if(objectUris[i].length() == 0){
                itemType = Consts.FOLDERS;
            }
            MediaItem mi = new MediaItem(objectNames[i],objectUris[i],thumbUris[i],itemType,objectIds[i],false);
            mi.Duration = gJNIInterface.getDuration(objectIds[i]);
            mediaItemArray.add(mi);
        }
        if(Consts.MUSIC == objectType && gMusicFragment != null){
            gMusicFragment.addObjects(mediaItemArray);
        }else if(Consts.VIDEOS == objectType && gVideosFragment != null){
            gVideosFragment.addObjects(mediaItemArray);
        }else if(Consts.PICTURES == objectType && gImagesFragment != null){
            gImagesFragment.addObjects(mediaItemArray);
        } 
    }
}
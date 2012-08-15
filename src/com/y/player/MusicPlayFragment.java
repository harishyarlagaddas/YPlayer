package com.y.player;

import android.app.Activity;
import android.app.Fragment;
import android.content.res.Configuration;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.media.MediaPlayer;
import android.media.MediaPlayer.OnCompletionListener;
import android.media.MediaPlayer.OnErrorListener;
import android.media.MediaPlayer.OnPreparedListener;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.ImageButton;
import android.widget.ImageView;
import android.widget.SeekBar;
import android.widget.SeekBar.OnSeekBarChangeListener;
import android.widget.TextView;
import android.widget.Toast;

import java.io.IOException;
import java.util.ArrayList;


public class MusicPlayFragment extends Fragment implements Runnable {
    private final String TAG = Consts.TAG + "-MusicPlayFragment";
    
    private Thread gProgressUpdateThread = null;
    private SeekBar gProgressBar = null;
    private ImageButton gPlayButton = null;
    private ImageButton gNextButton = null;
    private ImageButton gPrevButton = null;
    private ImageView gAlbumArt = null;
    private TextView gTitle = null;
    
    private ArrayList<MediaItem> gList = null;
    private int gCurrentIndex = 0;

    private Activity gActivity = null;
    
    private MediaPlayer gMediaPlayer = null;
    private boolean gIsPlayPending = false;
    private boolean gIsPlaying = false;
    private boolean gFragmentPaused = false;
    
    private int gDuration = 0;
    private int gCurrentProgress = 0;
    public MusicPlayFragment(){
        
    }
    
    public void onAttach (Activity activity){
        Log.i(TAG, "onAttach IN");
        super.onAttach(activity);
    }
    
    public void onCreate (Bundle savedInstanceState){
        Log.i(TAG, "onCreate IN");
        super.onCreate(savedInstanceState);
    }
    
    public View onCreateView (LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState){
        Log.i(TAG, "onCreateView IN");
        //super.onCreateView(inflater, container, savedInstanceState);
        View v =  inflater.inflate(R.layout.music_play, container,false);
        gActivity = getActivity();
        //gList = new ArrayList<MediaItem>();
        gMediaPlayer = new MediaPlayer();
        gProgressBar = (SeekBar)v.findViewById(R.id.progress);
        gProgressBar.setOnSeekBarChangeListener(new OnSeekBarChangeListener(){
            public void onProgressChanged(SeekBar sBar, int progress, boolean fromTouch) {
                if(fromTouch && null != gMediaPlayer && gIsPlaying){
                    Log.d(TAG,"MusicMainActivity:onProgressChanged: Seeking the MusicPlayer to the position: "+progress);
                    gMediaPlayer.seekTo(progress);
                }
            }
            public void onStartTrackingTouch(SeekBar arg0) {
                // TODO Auto-generated method stub
                
            }
            public void onStopTrackingTouch(SeekBar arg0) {
                // TODO Auto-generated method stub
                
            }
        });
        gPlayButton = (ImageButton) v.findViewById(R.id.play);
        gPrevButton  = (ImageButton) v.findViewById(R.id.previoussong);
        gNextButton = (ImageButton) v.findViewById(R.id.nextsong);
        
        gAlbumArt = (ImageView)v.findViewById(R.id.albumart);
        gTitle = (TextView)v.findViewById(R.id.musictitle);
        return v;
    }
    
    public void onStart (){
        Log.i(TAG, "onStart IN");
        super.onStart();
        
        if(gIsPlayPending){
            gIsPlayPending = false;
            playSong(gList.get(gCurrentIndex));
        }
    }
    
    public void onConfigurationChanged (Configuration newConfig){
        Log.i(TAG, "onConfigurationChanged IN");
        super.onConfigurationChanged(newConfig);
    }
    
    public void onPause(){
        super.onPause();
        Log.i(TAG,"onPause IN");
        gFragmentPaused = true;
        if(null != gProgressUpdateThread){
            gProgressUpdateThread.interrupt();
        }
    }
    
    public void onResume (){
        super.onResume();
        Log.i(TAG,"onResume IN");
        if(gIsPlaying && gFragmentPaused){
            gProgressUpdateThread = new Thread(this);
            gProgressUpdateThread.start();
        }
        gFragmentPaused = false;
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
    
    public boolean isPlaying(){
        return gIsPlaying;
    }
    
    public void stopProgressBarUpdate(){
        if(null != gProgressUpdateThread){
            gProgressUpdateThread.interrupt();
        }
    }
    
    public void startProgressBarUpdate(){
        if(gIsPlaying && null != gProgressUpdateThread){
            gProgressUpdateThread = new Thread(this);
            gProgressUpdateThread.start();
        }
    }
    
    public void playSongFromList(ArrayList<MediaItem> list, int index){
        if(gList == null){
            Log.e(TAG, "gList is NULL means this fragment has not completed the start.");
            gList = new ArrayList<MediaItem>();
            gIsPlayPending = true;
        }
        gList.clear();
        for(int i=0; i<list.size(); i++){
            MediaItem mi = list.get(i); 
            Log.i(TAG,"Adding the item: "+mi.Name+" to gList");
            gList.add(mi);
        }
        gCurrentIndex = index;
        if(false == gIsPlayPending){
            playSong(gList.get(index));
        }
    }
    
    private void setPlayingSongParams(final MediaItem mi){
        if(gActivity != null){
            gActivity.runOnUiThread(new Runnable(){
                public void run() {
                    if(gTitle != null){
                        gTitle.setText(mi.Name);
                    }
                    if(gAlbumArt != null){
                        String tPath = ImageLoader.getCachedFileName(mi.ThumbnailPath);
                        Bitmap b = null;
                        if(tPath != null){
                            b = BitmapFactory.decodeFile(tPath);
                        }else{
                            b = BitmapFactory.decodeResource(gActivity.getResources(), R.drawable.default_audio);
                        }
                        gAlbumArt.setImageBitmap(b);
                    }
                }
            });
        }
    }
    
    private void playSong(MediaItem mi){
        String path = mi.Path;
        if(null == gMediaPlayer || null == path){
            Log.e(TAG, "Error!! gMediaPlayer is NULL which should not happen!!");
            return;
        }
        
        if(path.startsWith("http")){
        }
        
        try {
            clearLocalData();
            gDuration = mi.Duration;
            Log.d(TAG,"Initializing the MediaPlayer to play the song: "+path);
            gMediaPlayer.reset();
            gIsPlaying = true;
            setPlayingSongParams(mi);
            gMediaPlayer.setDataSource(path);
            gMediaPlayer.prepare();
            gMediaPlayer.start();
            gMediaPlayer.setOnPreparedListener(new OnPreparedListener(){
                public void onPrepared(MediaPlayer arg0) {
                    Log.i(TAG,"onPrepared Callback from Music Player");
                    if(0 == gDuration){
                        gDuration = gMediaPlayer.getDuration();
                        Log.i(TAG, "Setting the Max Duration as: "+gDuration);
                        gProgressBar.setMax(gDuration);
                    }
                }
            });
            gMediaPlayer.setOnCompletionListener(new OnCompletionListener(){
                public void onCompletion(MediaPlayer arg0) {
                    Log.i(TAG,"onCompletion Callback from Music Player");
                    clearLocalData();
                    if(gProgressUpdateThread != null){
                        Log.i(TAG,"Interupting the Progress Update Thread");
                        gProgressUpdateThread.interrupt();
                    }
                    gMediaPlayer.stop();
                    gCurrentIndex++;
                    if(gCurrentIndex < gList.size()){
                        new Thread(new Runnable(){
                            public void run() {
                                playSong(gList.get(gCurrentIndex));
                            }
                        }).start();
                    }else{
                        gCurrentIndex--;
                    }
                }
            });
            gMediaPlayer.setOnErrorListener(new OnErrorListener(){
                public boolean onError(MediaPlayer arg0, int arg1, int arg2) {
                    Log.i(TAG,"onError Callback from Music Player");
                    clearLocalData();
                    if(gProgressUpdateThread != null){
                        Log.i(TAG,"Interupting the Progress Update Thread");
                        gProgressUpdateThread.interrupt();
                    }
                    gMediaPlayer.stop();
                    if(gActivity != null){
                        gActivity.runOnUiThread (new Runnable () {
                            public void run () {
                                Toast.makeText(gActivity, gActivity.getString(R.string.play_err), Toast.LENGTH_SHORT).show();
                            }
                        });
                    }
                    gCurrentIndex++;
                    if(gCurrentIndex < gList.size()){
                        new Thread(new Runnable(){
                            public void run() {
                                playSong(gList.get(gCurrentIndex));
                            }
                        }).start();
                    }else{
                        gCurrentIndex--;
                    }
                    return true;
                }
            });
            gProgressBar.setProgress(0);
            gProgressBar.setMax(gDuration);
            gProgressUpdateThread = new Thread(this);
            gProgressUpdateThread.start();
            
            if(gPlayButton != null){
                gPlayButton.setOnClickListener(new OnClickListener(){
                    public void onClick(View arg0) {
                        if(gMediaPlayer.isPlaying()){
                            gMediaPlayer.pause();
                            gPlayButton.setImageResource(R.drawable.play);
                        }else if(gIsPlaying){
                            gMediaPlayer.start();
                            gPlayButton.setImageResource(R.drawable.pause);
                        }else{
                            if(null != gList && gCurrentIndex < gList.size()){
                                new Thread(new Runnable(){
                                    public void run() {
                                        playSong(gList.get(gCurrentIndex));
                                    }
                                }).start();
                            }
                        }
                    }
                });
            }
            
            if(gPrevButton != null){
                gPrevButton.setOnClickListener(new OnClickListener(){
                    public void onClick(View v) {
                        gCurrentIndex--;
                        if(gCurrentIndex >= 0){
                            new Thread(new Runnable(){
                                public void run() {
                                    playSong(gList.get(gCurrentIndex));
                                }
                            }).start();
                        }else{
                            gCurrentIndex++;
                            if(gActivity != null){
                                gActivity.runOnUiThread (new Runnable () {
                                    public void run () {
                                        Toast.makeText(gActivity, gActivity.getString(R.string.end_list), Toast.LENGTH_SHORT).show();
                                    }
                                });
                            }
                        }
                    }
                });
            }
            
            if(gNextButton != null){
                gNextButton.setOnClickListener(new OnClickListener(){
                    public void onClick(View v) {
                        gCurrentIndex++;
                        if(gCurrentIndex < gList.size()){
                            new Thread(new Runnable(){
                                public void run() {
                                    playSong(gList.get(gCurrentIndex));
                                }
                            }).start();
                        }else{
                            gCurrentIndex--;
                            if(gActivity != null){
                                gActivity.runOnUiThread (new Runnable () {
                                    public void run () {
                                        Toast.makeText(gActivity, gActivity.getString(R.string.end_list), Toast.LENGTH_SHORT).show();
                                    }
                                });
                            }
                        }
                    }
                });
            }
        } catch (IllegalArgumentException e) {
            Log.e(TAG,"Exception while setting and starting the MediaPlayer");
            clearLocalData();
            e.printStackTrace();
        } catch (SecurityException e) {
            Log.e(TAG,"Exception while setting and starting the MediaPlayer");
            clearLocalData();
            e.printStackTrace();
        } catch (IllegalStateException e) {
            Log.e(TAG,"Exception while setting and starting the MediaPlayer");
            clearLocalData();
            e.printStackTrace();
        } catch (IOException e) {
            Log.e(TAG,"Exception while setting and starting the MediaPlayer");
            clearLocalData();
            e.printStackTrace();
        }
    }
    
    public void run() {
        Log.i(TAG, "Progress Update Thread Start IN");
        if(null == gMediaPlayer || null == gProgressBar){
            Log.e(TAG, "Something wrong in the Progressbar Update Thread. Hence exiting from this thread");
            return;
        }
        Log.i(TAG, "Setting the Max Duration as: "+gDuration);
        do{
            try {
                Thread.sleep(1000);
                gCurrentProgress= gMediaPlayer.getCurrentPosition();
            } catch (InterruptedException e) {
                return;
            } catch (Exception e){
                return;
            }
            gActivity.runOnUiThread (new Runnable () {
                public void run () {
                    Log.i(TAG,"MusicMainActivity:run: Setting the progress as "+gCurrentProgress);
                    if(gProgressBar != null){
                        gProgressBar.setProgress(gCurrentProgress);
                        //gProgressBar.setText(Utils.ConvertDurationToString(gCurrentProgress)+"/"+Utils.ConvertDurationToString(gDuration));
                    }
                }
            });
            if(Thread.interrupted()){
                Log.i(TAG,"Progress Update Thread got Interrupted!");
                break;
            }
        }while(gCurrentProgress > 0);
        Log.i(TAG, "Progress Update Thread is Ending!");
    }
    
    public void clearLocalData(){
        gCurrentProgress = 0;
        gDuration = 0;
        gIsPlaying = false;
    }
}

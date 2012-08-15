package com.y.player;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.util.ArrayList;

import android.app.Activity;
import android.app.ProgressDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.DialogInterface.OnKeyListener;
import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.os.Bundle;
import android.view.GestureDetector;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;
import android.widget.ImageView;
import android.widget.ProgressBar;
import android.widget.Toast;


public class ImageViewer extends Activity {
    private String TAG = Consts.TAG+"-ImageViewer";
    
    private Bitmap gBitmap = null;
    private ImageView gImageView = null;
    private ProgressBar gProgressbar = null;
    private ArrayList<String> gPictureList = null;
    private int gCurrentIndex = 0;
    private GestureDetector gGestureDetector = null;
    private Context gContext = null;
    private ProgressDialog gPD = null;
    
    private boolean gIsStartingPicture = false;
    private boolean gIsDecodingPicture = false;
    
    /** Called when the activity is first created. */
    public void onCreate(Bundle savedInstanceState) {
        Log.i(TAG, "onCreate IN");
        super.onCreate(savedInstanceState);
        
        //Remove title bar
        this.requestWindowFeature(Window.FEATURE_NO_TITLE);
        //Remove notification bar
        this.getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN, WindowManager.LayoutParams.FLAG_FULLSCREEN);
        
        setContentView(R.layout.image_view);
        gImageView = (ImageView)findViewById(R.id.image);
        gProgressbar = (ProgressBar)findViewById(R.id.progress);
        gContext = this;
        Intent i = getIntent();
        if(i.hasExtra(Consts.PICTURE_LIST)){
            gPictureList = i.getStringArrayListExtra(Consts.PICTURE_LIST);
        }
        if(i.hasExtra(Consts.PICTURE_LIST_INDEX)){
            gCurrentIndex = i.getIntExtra(Consts.PICTURE_LIST_INDEX, 0);
        }
        if(null != gPictureList && gCurrentIndex >= 0 && gCurrentIndex < gPictureList.size()){
            //showProgressDialog(getString(R.string.image_get));
            gIsStartingPicture = true;
            new Thread(new Runnable(){
                public void run() {
                    displayPicture(gPictureList.get(gCurrentIndex));     
                }
            }).start();
        }else{
            Log.e(TAG, "Name is NULL. Hence can not do any thing");
            Toast.makeText(gContext, gContext.getString(R.string.image_err), Toast.LENGTH_SHORT).show(); 
            finish();
        }
        
        gGestureDetector = new GestureDetector(this, new GestureDetector.SimpleOnGestureListener() {

            public boolean onFling(MotionEvent e1, MotionEvent e2,final float velocityX,final float velocityY) {
                int dx = (int) (e2.getX() - e1.getX());
                // don't accept the fling if it's too short
                // as it may conflict with a button push
                if (Math.abs(dx) > 150 && Math.abs(velocityX) > Math.abs(velocityY)) {
                    if (velocityX > 0) {
                        moveRight();
                    } else {
                        moveLeft();
                    }
                    return true;
                } else {
                    return false;
                }
            }
        });
        
    }
    
    public void onDestroy(){
        super.onDestroy();
        if(gBitmap != null){
            gBitmap.recycle();
            gBitmap = null;
        }
        if(null != gPD){
            gPD.dismiss();
        }
    }
    
    @Override
    public boolean dispatchTouchEvent(MotionEvent event) {
        //Log.i(TAG, "dispatchTouchEvent");

        if(gGestureDetector.onTouchEvent(event))
            return true;
        else
        return super.dispatchTouchEvent(event);

    }
    
    public void moveLeft(){
        if(gIsDecodingPicture){
            Log.d(TAG, "Presently decoding one picture to show it on screen. Meanwhile swiping to see the other picture doesn't make sencse! Please be patient!");
            return;
        }
        gIsStartingPicture = false;
        gCurrentIndex++;
        if(null != gPictureList && gCurrentIndex < gPictureList.size()){
            //showProgressDialog(getString(R.string.image_get));
            new Thread(new Runnable(){
                public void run() {
                    displayPicture(gPictureList.get(gCurrentIndex));
                }
            }).start();
        }else{
            gCurrentIndex--;
            runOnUiThread(new Runnable(){
                public void run() {
                    Toast.makeText(gContext, getString(R.string.image_end), Toast.LENGTH_SHORT).show();
                }
            });
        }
    }
    
    public void moveRight(){
        if(gIsDecodingPicture){
            Log.d(TAG, "Presently decoding one picture to show it on screen. Meanwhile swiping to see the other picture doesn't make sencse! Please be patient!");
            return;
        }
        gIsStartingPicture = false;
        gCurrentIndex--;
        if(null != gPictureList && gCurrentIndex >= 0){
            //showProgressDialog(getString(R.string.image_get));
            new Thread(new Runnable(){
                public void run() {
                    displayPicture(gPictureList.get(gCurrentIndex));
                }
            }).start();
        }else{
            gCurrentIndex++;
            runOnUiThread(new Runnable(){
                public void run() {
                    Toast.makeText(gContext, getString(R.string.image_end), Toast.LENGTH_SHORT).show();
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
                        gPD.setOnKeyListener(new OnKeyListener(){
                            public boolean onKey(DialogInterface arg0, int arg1, KeyEvent arg2) {
                                if (arg1 == KeyEvent.KEYCODE_BACK || arg1 == KeyEvent.KEYCODE_HOME) {
                                    Log.d(TAG, "Back/Home Key Pressed. Hence cancelling Progress Dialog");
                                    //dismissProgressDialog();
                                    if(gIsStartingPicture){
                                        finish();
                                    }
                                }
                                return true;
                            }
                        });
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
    
    public void displayPicture(String name){
        gIsDecodingPicture = true;
        runOnUiThread(new Runnable(){
            public void run() {
                gProgressbar.setVisibility(View.VISIBLE);
                gImageView.setVisibility(View.INVISIBLE);
            }
        });
        
        if(null != gBitmap && !gBitmap.isRecycled()){
            gBitmap.recycle();
            gBitmap = null;
        }
        File file= null;
        FileInputStream fs=null;
        try{
        	BitmapFactory.Options bfOptions=new BitmapFactory.Options();
            bfOptions.inDither=false;                     
            bfOptions.inPurgeable=true;                   
            bfOptions.inInputShareable=true;
            
	        if(name.startsWith("http")){
	            file = new File(ImageLoader.GetCacheDir(),String.valueOf(name.toUpperCase().hashCode()));
	            if(!file.exists()){
	                Utils.downloadFile(file.getAbsolutePath(),name);
	            }
	            //gBitmap = BitmapFactory.decodeFile(f.getAbsolutePath());
	            fs = new FileInputStream(file);
	            gBitmap = BitmapFactory.decodeFileDescriptor(fs.getFD(), null, bfOptions);
	        }else{
	        	file = new File(name);
	        	fs = new FileInputStream(file);
	            gBitmap = BitmapFactory.decodeFileDescriptor(fs.getFD(), null, bfOptions);
	            gBitmap = BitmapFactory.decodeFile(name);
	        }
        }catch(java.lang.OutOfMemoryError e){
        	Log.e(TAG, "Error Out Of Memory Error!!");
        } catch (FileNotFoundException e) {
        	Log.e(TAG, "Error File Not Found Error!!");
		} catch (IOException e) {
			Log.e(TAG, "Error IO Exception!!");
		}
        
        try{
        	if(null != fs){
        		fs.close();
        	}
        }catch(IOException e){
            Log.e(TAG, "Failded to close the file stream!!");
        }
        
        //dismissProgressDialog();
        if(null != gBitmap && null != gImageView){
            runOnUiThread(new Runnable(){
                public void run() {
                    gImageView.setImageBitmap(gBitmap);
                    gProgressbar.setVisibility(View.INVISIBLE);
                    gImageView.setVisibility(View.VISIBLE);
                }
            });
        }else{
            Log.e(TAG, "Decoded Bitmap is NULL. Hence can not do any thing");
            gBitmap = BitmapFactory.decodeResource(getResources(), R.drawable.broken_image);
            runOnUiThread(new Runnable(){
                public void run() {
                    Toast.makeText(gContext, gContext.getString(R.string.image_err), Toast.LENGTH_SHORT).show();
                    if(null != gImageView){
	                    gImageView.setImageBitmap(gBitmap);
	                    gProgressbar.setVisibility(View.INVISIBLE);
	                    gImageView.setVisibility(View.VISIBLE);
                    }
                }
            });
            //finish();
        }
        gIsDecodingPicture = false;
    }
}

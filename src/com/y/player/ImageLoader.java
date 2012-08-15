package com.y.player;

import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileDescriptor;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.net.MalformedURLException;
import java.net.URL;
import java.util.HashMap;
import java.util.Stack;

import android.app.Activity;
import android.content.ContentResolver;
import android.content.Context;
import android.content.res.Resources;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.net.Uri;
import android.os.ParcelFileDescriptor;
import android.provider.MediaStore;
import android.view.View;
import android.widget.ImageView;
import android.widget.ProgressBar;

public class ImageLoader {
    public static final String TAG = Consts.TAG+"-ImageLoader";
    //the simplest in-memory cache implementation. This should be replaced with something like SoftReference or BitmapOptions.inPurgeable(since 1.6)
    private HashMap<String, Bitmap> cache=new HashMap<String, Bitmap>();

    ContentResolver gResolver = null;
    Resources gResources = null;
    //Task for the queue
    private class PhotoToLoad
    {
        public String url;
        public int contentType;
        public boolean LocalType;
        public ImageView imageView;
        public ProgressBar progress;
        public PhotoToLoad(final String u, final int t, final boolean localItem, final ImageView i, ProgressBar pb){
            url=u; 
            contentType = t;
            LocalType = localItem;
            imageView=i;
            progress = pb;
        }
    }
    
    class PhotosLoader extends Thread {
        public void run() {
            try {
                while(true)
                {
                    //thread waits until there are any images to load in the queue
                    if(photosQueue.photosToLoad.size()==0){
                        synchronized(photosQueue.photosToLoad){
                            photosQueue.photosToLoad.wait();
                        }
                    }
                    
                    if(photosQueue.photosToLoad.size()!=0)
                    {
                        PhotoToLoad photoToLoad;
                        synchronized(photosQueue.photosToLoad){
                            photoToLoad=photosQueue.photosToLoad.pop();
                        }
                        //Log.i(TAG,"ImageLoader:PhotosLoader getting the Bitmap for url = " +photoToLoad.url);
                        Bitmap bmp = null;
                        /* In case of Audio Album art we need to get the album art in a different way on Android. And do
                         * remember that this is only for getting the album art for localdevice. Not for remote device..
                         */
                        if(photoToLoad.LocalType){
                            if(photoToLoad.contentType == Consts.MUSIC &&
                                    photoToLoad.url.startsWith("content://media/external/audio/albumart")){
                                //Log.i(TAG,"ImageLoader:PhotoLoader getting the album art for songs on Android device");
                                Uri uri = Uri.parse(photoToLoad.url);
                                try{
                                    ParcelFileDescriptor pfd = gResolver.openFileDescriptor(uri, "r");
                                    if (pfd != null) {
                                        FileDescriptor fd = pfd.getFileDescriptor();
                                        bmp = BitmapFactory.decodeFileDescriptor(fd);
                                    }else{
                                        bmp = BitmapFactory.decodeResource(gResources, R.drawable.default_audio);
                                        //Log.e(TAG,"ImageLoader:PhotoLoader failed to decode the albumart resource. Hence setting default albumart");
                                    }
                                }catch(Exception e){
                                    bmp = BitmapFactory.decodeResource(gResources, R.drawable.default_audio);
                                    //Log.e(TAG,"ImageLoader:PhotoLoader Got Exception while getting the album art for audio songs on Android device");
                                }
                            }else if(photoToLoad.contentType == Consts.VIDEOS){
                                int origID = Integer.parseInt(photoToLoad.url);
                                bmp = MediaStore.Video.Thumbnails.getThumbnail(gResolver, origID, MediaStore.Images.Thumbnails.MINI_KIND, null);
                            }else if(photoToLoad.contentType == Consts.PICTURES){
                                int origID = Integer.parseInt(photoToLoad.url);
                                bmp = MediaStore.Images.Thumbnails.getThumbnail(gResolver, origID, MediaStore.Images.Thumbnails.MINI_KIND, null);
                            }
                        }else{
                            bmp=getBitmap(photoToLoad.url);
                        }
                        //cache.put(photoToLoad.url, bmp);
                        BitmapDisplayer bd=new BitmapDisplayer(bmp, photoToLoad.imageView,photoToLoad.contentType, photoToLoad.progress);
                        Activity a=(Activity)photoToLoad.imageView.getContext();
                        //Log.d(TAG,"ImageLoader:PhotosLoader loading the image on UI Thread");
                        a.runOnUiThread(bd);
                    }
                    if(Thread.interrupted())
                        break;
                }
            } catch (InterruptedException e) {
                //allow thread to exit
            }
        }
    }
    
    //Used to display bitmap in the UI thread
    class BitmapDisplayer implements Runnable
    {
        Bitmap bitmap;
        ImageView imageView;
        ProgressBar progress;
        int type;
        public BitmapDisplayer(Bitmap b, ImageView i,int t, ProgressBar pb){bitmap=b;imageView=i;type=t;progress = pb;}
        public void run()
        {
            if(bitmap!=null){
                //Log.i(TAG,"ImageLoader:BitmapDisplayer Loading the Image with correct BitMap");
                imageView.setImageBitmap(bitmap);
            }
            else{
                //Log.e(TAG,"ImageLoader:BitmapDisplayer Loading the Image with correct BitMap Failed. Hence loading with broken image");
                imageView.setImageResource(GetImageResourceId(type));
            }
            if(null != progress){
                progress.setVisibility(View.INVISIBLE);
            }
            imageView.setVisibility(View.VISIBLE);
        }
    }
    
    //stores list of photos to download
    class PhotosQueue
    {
        private Stack<PhotoToLoad> photosToLoad=new Stack<PhotoToLoad>();
        
        //removes all instances of this ImageView
        public void Clean(ImageView image)
        {
            for(int j=0 ;j<photosToLoad.size();){
                try{
                    synchronized(photosQueue.photosToLoad){
                        if(photosToLoad.get(j).imageView==image)
                            photosToLoad.remove(j);
                        else
                            ++j;
                    }
                }catch(java.lang.ArrayIndexOutOfBoundsException e){
                    Log.d(TAG,"Got Array Out Of Bound Exception while cleaning the photos queue. Ignoring it temporarily");
                }
            }
        }
    }
    
    private static File cacheDir;
    PhotosLoader photoLoaderThread=new PhotosLoader();
    PhotosQueue photosQueue=new PhotosQueue();
    
    public ImageLoader(Context context){
        Log.d(TAG,"ImageLoader:ImageLoader IN");
        //Make the background thread low priority. This way it will not affect the UI performance
        photoLoaderThread.setPriority(Thread.NORM_PRIORITY-1);
        
        gResources = context.getResources(); 
        CharSequence appName = "." + context.getString(R.string.app_name);
        gResolver = context.getContentResolver();
        
        //Find the dir to save cached images
        if (android.os.Environment.getExternalStorageState().equals(android.os.Environment.MEDIA_MOUNTED))
            cacheDir=new File(android.os.Environment.getExternalStorageDirectory(),appName.toString());
        else
            cacheDir=context.getCacheDir();
        
        if(!cacheDir.exists()){
            Log.i(TAG,"ImageLoader:ImageLoader Cached Dir = " +cacheDir.getPath() +" does not exist. Hence Creating the new one");
            if(cacheDir.mkdir()){
                Log.i(TAG,"ImageLoader:ImageLoader Successfully Create the Cached Dir = " +cacheDir.getPath());
            }else{
                Log.e(TAG,"ImageLoader:ImageLoader Failed to Create the Cached Dir = " +cacheDir.getPath());
            }
        }
    }
    
    public void DisplayImage(String url, int contentType, boolean localItem, Activity activity, ImageView imageView, ProgressBar pb)
    {
        //if(url != null) Log.d(TAG,"ImageLoader:DisplayImage: IN url = "+url);
        if(null == url || 0 == url.length()){
            //Log.d(TAG,"ImageLoader:DisplayImage: thumbnail url is null. Hence setting the default image");
            imageView.setImageResource(GetImageResourceId(contentType));
            if(null != pb){
                pb.setVisibility(View.INVISIBLE);
            }
            imageView.setVisibility(View.VISIBLE);
        }else if(cache.containsKey(url) && null != cache.get(url)){
            //Log.d(TAG,"ImageLoader:DisplayImage: Got the image in Cache. So setting it here itself");
            imageView.setImageBitmap(cache.get(url));
            if(null != pb){
                pb.setVisibility(View.INVISIBLE);
            }
            imageView.setVisibility(View.VISIBLE);
        }else{
            imageView.setVisibility(View.INVISIBLE);
            if(null != pb){
                pb.setVisibility(View.VISIBLE);
            }
            //Log.d(TAG,"ImageLoader:DisplayImage: Querying for Image and temporarily setting LOADING Image");
            queuePhoto(url, contentType, localItem, activity, imageView,pb);
            //imageView.setImageResource(R.drawable.loading);
        }    
    }
        
    public int GetImageResourceId(int type){
        if(Consts.MUSIC == type){
        	//Log.i(TAG,"ImageLoader:GetImageResourceId: returning default Music resource");
            return R.drawable.default_audio;
        }else if(Consts.VIDEOS == type){
        	//Log.i(TAG,"ImageLoader:GetImageResourceId: returning default Video resource");
            return R.drawable.default_video;
        }else if(Consts.PICTURES == type){
        	//Log.i(TAG,"ImageLoader:GetImageResourceId: returning default Picture resource");
            return R.drawable.default_picture;
        }else if(Consts.FOLDERS == type){
            return R.drawable.folder;
        }
        //Log.i(TAG,"ImageLoader:GetImageResourceId: returning default Picture resource as last option");
        return R.drawable.default_picture;
    }
    private void queuePhoto(String url, int contentType, boolean localItem, Activity activity, ImageView imageView, ProgressBar pb)
    {
        //Log.i(TAG,"ImageLoader:queuePhoto: IN");
        //This ImageView may be used for other images before. So there may be some old tasks in the queue. We need to discard them. 
        photosQueue.Clean(imageView);
        PhotoToLoad p=new PhotoToLoad(url, contentType, localItem, imageView, pb);
        synchronized(photosQueue.photosToLoad){
            //Log.d(TAG,"ImageLoader:queuePhoto: pushing the image to photosQueue");
            photosQueue.photosToLoad.push(p);
            photosQueue.photosToLoad.notifyAll();
        }
        //start thread if it's not started yet
        if(photoLoaderThread.getState()==Thread.State.NEW){
            Log.d(TAG,"ImageLoader:queuePhoto: Starting photoLoaderThread");
            photoLoaderThread.start();
        }
    }
    
    private Bitmap getBitmap(String path) 
    {
        Log.i(TAG,"ImageLoader:getBitmap: IN URL = "+path);
        String filename=String.valueOf(path.hashCode());
        File f=new File(cacheDir, filename);
        File f1 = new File(cacheDir,String.valueOf(path.toUpperCase().hashCode()));
        Log.d(TAG,"CacheDir: "+cacheDir);
        /* In case if the original file is already downloaded by Viewing the image, then use the original
         * image instead of thumbnail so that the picture will be seen more clearly.
         */
        if(f1.exists()){
            return decodeFile(f1);   
        }else if(!f.exists() && path.startsWith("http")){
            downloadFile(path,f.getAbsolutePath());
        }
        return decodeFile(f);
    }

    public static String getCachedFileName(String path){
        String filename=String.valueOf(path.hashCode());
        File f=new File(cacheDir, filename);
        if(f.exists()){
            return f.getAbsolutePath();
        }
        return null;
    }
    
    public void downloadFile(String inFile, String outFilePath){
        InputStream is = null;
        FileOutputStream fos = null;
        BufferedOutputStream bos = null;
        URL url = null;
        try {
            url = new URL(inFile);
            is = url.openStream();
            File outfile = new File (outFilePath);
            if(outfile.exists()){
                outfile.delete();
            }
            outfile.createNewFile ();
            fos = new FileOutputStream(outfile);
            bos = new BufferedOutputStream(fos);
            byte [] buf = new byte[1024];
            int temp = 0;
            while((temp = is.read (buf)) > 0) {
                bos.write(buf,0,temp);
            }
        } catch (MalformedURLException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        } catch (IOException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }
        try{
            if(null != is){
                is.close();
            }
            if(null != bos){
                bos.close();
            }
            if(null != fos){
                fos.close();
            }
        }catch(IOException e){
            e.printStackTrace();
        }
    }
    
    //decodes image and scales it to reduce memory consumption
    private Bitmap decodeFile(File f){
        if(!f.exists()){
            //Log.e(TAG,"ImageLoader:decodeFile: File Not Found. Returning NULL");
            return null;
        }
        try {
            //decode image size
            BitmapFactory.Options o = new BitmapFactory.Options();
            o.inJustDecodeBounds = true;
            BitmapFactory.decodeStream(new FileInputStream(f),null,o);
            
            final int REQUIRED_SIZE=70;
            int width_tmp=o.outWidth, height_tmp=o.outHeight;
            int scale=1;
            while(true){
                if(width_tmp/2<REQUIRED_SIZE || height_tmp/2<REQUIRED_SIZE)
                    break;
                width_tmp/=2;
                height_tmp/=2;
                scale*=2;
            }
            BitmapFactory.Options o2 = new BitmapFactory.Options();
            o2.inSampleSize=scale;
            return BitmapFactory.decodeStream(new FileInputStream(f), null, o2);
        } catch (FileNotFoundException e) {
            Log.e(TAG,"ImageLoader:decodeFile: Exception caught while decoding the file.");
        }
        return null;
    }
    
    public void stopThread()
    {
        Log.i(TAG,"ImageLoader:stopThread: IN stopping the photoLoaderThread");
        photoLoaderThread.interrupt();
        clearMemoryCache();
        clearFileCache();
    }

    public void clearMemoryCache() {
        Log.i(TAG,"ImageLoader:clearMemoryCache: IN Clearing the cache");
        //clear memory cache
        cache.clear();
    }
    
    public static void clearFileCache() {
        Log.i(TAG,"ImageLoader:clearFileCache: IN Clearing the cache");
        //clear SD cache
        File[] files=cacheDir.listFiles();
        for(File f:files)
            f.delete();
    }
    
    public static String GetCacheDir(){
        return cacheDir.getPath();
    }

}

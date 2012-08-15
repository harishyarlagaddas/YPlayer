package com.y.player;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;

public class Log {
    private static int LOG_LEVEL_LOW = 3;
    private static int LOG_LEVEL_MEDIUM = 2;
    private static int LOG_LEVEL_HIGH = 1;
    
    private static int mLogLevel = LOG_LEVEL_LOW;
    
    static boolean LOG = false;
    static FileOutputStream gFileOut = null;
    static File gFile = null;
    
    public static void InitLog(){
        File f = new File("/sdcard/",Consts.LOG_FILE_CHECK);
        if(f.exists()){
            LOG = true;
            gFile = new File("/sdcard/",Consts.LOG_FILE_NAME);
            try {
                gFileOut = new FileOutputStream(gFile);
            } catch (FileNotFoundException e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            }
        }else{
            LOG = false;
            if(gFileOut != null){
                try {
                    gFileOut.close();
                } catch (IOException e) {
                    // TODO Auto-generated catch block
                    e.printStackTrace();
                }
                gFileOut = null;
            }
        }
    }
    
    public static void CloseLog(){
        if(gFileOut != null){
            try {
                gFileOut.close();
                gFileOut = null;
            } catch (IOException e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            }
        }
    }
    
    public static void DeleteLog(){
        CloseLog();
        File f = new File("/sdcard/",Consts.LOG_FILE_NAME);
        if(f.exists()){
            f.delete();
        }
        
        f = new File("/sdcard/",Consts.LOG_FILE_CHECK);
        if(f.exists()){
            f.delete();
        }
    }

    public static void i(String tag, String string) {
        if(mLogLevel >= LOG_LEVEL_LOW){
            android.util.Log.i(tag,string);
        }
        if(gFileOut != null){
            try {
                gFileOut.write(string.getBytes());
                gFileOut.write("\n".getBytes());
            } catch (IOException e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            }
        }
    }
    public static void e(String tag, String string) {
        if(mLogLevel >= LOG_LEVEL_HIGH){
            android.util.Log.e(tag,string);
        }
        if(gFileOut != null){
            try {
                gFileOut.write(string.getBytes());
                gFileOut.write("\n".getBytes());
            } catch (IOException e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            }
        }
    }
    public static void d(String tag, String string) {
        if(mLogLevel >= LOG_LEVEL_MEDIUM){
            android.util.Log.d(tag,string);
        }
        if(gFileOut != null){
            try {
                gFileOut.write(string.getBytes());
                gFileOut.write("\n".getBytes());
            } catch (IOException e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            }
        }
    }
    public static void v(String tag, String string) {
        if (LOG) android.util.Log.v(tag, string);
        if(gFileOut != null){
            try {
                gFileOut.write(string.getBytes());
                gFileOut.write("\n".getBytes());
            } catch (IOException e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            }
        }
    }
    public static void w(String tag, String string) {
        if (LOG) android.util.Log.w(tag, string);
        if(gFileOut != null){
            try {
                gFileOut.write(string.getBytes());
                gFileOut.write("\n".getBytes());
            } catch (IOException e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            }
        }
    }
}

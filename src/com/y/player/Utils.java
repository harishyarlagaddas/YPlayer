package com.y.player;

import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.net.MalformedURLException;
import java.net.URL;
import java.util.Date;

public class Utils {
    public static String TAG = Consts.TAG + "-Utils";
    public static String ConvertDurationToString(long duration){
        long hr = 0, min = 0, sec = 0, dur = 0;
        StringBuilder duration1 = new StringBuilder();
        dur = duration/1000;
        hr = dur/(60*60);
        dur -= hr*(60*60);
        min = dur/60;
        dur -= min*60;
        sec = dur;
        if(9 > hr){
            duration1.append("0");
        }
        duration1.append(String.valueOf(hr));
        duration1.append(":");
        if(9 > min){
            duration1.append("0");
        }
        duration1.append(String.valueOf(min));
        duration1.append(":");
        if(9 > sec){
            duration1.append("0");
        }
        duration1.append(String.valueOf(sec));
        return duration1.toString();
    }
    
    public static String ConvertSizeToString(long size){
        long gb = 0, mb = 0, kb = 0, bytes = 0, s = size;
        String size1 = null;
        gb = s / 1000000000;
        s -= gb * 1000000000;
        mb = s / 1000000;
        s -= mb * 1000000;
        kb = s / 1000;
        s -= kb * 1000;
        bytes = s;
        if(gb > 0){
            size1 = String.valueOf(gb) + "GB";
        }else if(mb > 0){
            size1 = String.valueOf(mb) + "MB";
        }else if(kb > 0){
            size1 = String.valueOf(kb) + "KB";
        }else{
            size1 = String.valueOf(bytes) + "Bytes";
        }
        return size1;
    }
    
    public static String ConvertToDateFormat(long date){
        Date d = new Date(date);
        StringBuilder dateStr = new StringBuilder();
        if(9 > d.getMonth()){
            dateStr.append("0");
        }
            dateStr.append(d.getMonth());
        dateStr.append("/");
        if(9 > d.getDate()){
            dateStr.append("0");
        }
        dateStr.append(d.getDay());
        dateStr.append("/");
        dateStr.append(d.getYear());
        return dateStr.toString();
    }
    
    public static void downloadFile(String path, String url1){
        InputStream is = null;
        FileOutputStream fos = null;
        BufferedOutputStream bos = null;
        URL url = null;
        try {
            url = new URL(url1);
            is = url.openStream();
            File outfile = new File (path);
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
}

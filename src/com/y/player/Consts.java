package com.y.player;

public class Consts{
	public static final String TAG = "Y-Player";
	
	/* Type of the Items like Audio, Video, PIctures Documents, All Media e.t.c... */
	public static int MUSIC = 101;
	public static int PICTURES = 102;
	public static int VIDEOS = 103;
	public static int FOLDERS = 104;
	
	/* Server ID */
	public static int LOCAL_HOST = -1;
	
	/* Log File Names to write the logs */
	public static String LOG_FILE_CHECK = ".YPlayer";
	public static String LOG_FILE_NAME = ".YPlayer.log";
	
	public static int GET_NUMBER_OF_ELEMENTS_COUNT = 20;
	
	public static int MAIN_WINDOW = 201;
	public static int SUB_WINDOW = 202;
	
	public static String PICTURE_LIST = "PICTURE_LIST";
	public static String PICTURE_LIST_INDEX = "PICTURE_LIST_INDEX";
	public static String DOWNLOADS_DIRECTORY = "/Y-Player Downloads/";
	
	public static class ServerInfo{
	    String serverName;
	    int serverID;
	    
	    public ServerInfo(String sName, int sId){
	        serverName = sName;
	        serverID = sId;
	    }
	}
}
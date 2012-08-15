/*
 * Copyright (C) 2009 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */
#include <stdio.h>
#include <string.h>
#include <jni.h>
//#include <JNIHelp.h>
//#include <android_runtime/AndroidRuntime.h>
#include "android/log.h"

#include "common/log/log.h"

static JavaVM *Jvm = NULL;

static const char *gJniInterfaceClsName        = "com/y/player/JniInterface";
static jobject gJniInterfaceObj = NULL;

static jmethodID cbNewServerAdded = NULL;
static jmethodID cbNewRendererAdded = NULL;
static jmethodID cbNewServerRemoved = NULL;
static jmethodID cbNewRendererRemoved = NULL;
static jmethodID cbNotifyGetAllSongs = NULL;
static jmethodID cbNotifyGetAllVideos = NULL;
static jmethodID cbNotifyGetAllPictures = NULL;
static jmethodID cbNotifyGetMusicObjects = NULL;
static jmethodID cbNotifyGetVideoObjects = NULL;
static jmethodID cbNotifyGetPictureObjects = NULL;

JNIEXPORT void JNICALL Java_com_y_player_JniInterface_initNativeLog(JNIEnv* env, jobject obj);
JNIEXPORT void JNICALL Java_com_y_player_JniInterface_closeNativeLog(JNIEnv* env, jobject obj);
JNIEXPORT void JNICALL Java_com_y_player_JniInterface_startControlPoint(JNIEnv* env, jobject obj);
JNIEXPORT jint JNICALL Java_com_y_player_JniInterface_selectServer(JNIEnv* env, jobject obj, jint serverId);
JNIEXPORT void JNICALL Java_com_y_player_JniInterface_clearServerSelection(JNIEnv* env, jobject obj);
JNIEXPORT jint JNICALL Java_com_y_player_JniInterface_selectRenderer(JNIEnv* env, jobject obj, jint rendererId);
JNIEXPORT void JNICALL Java_com_y_player_JniInterface_getAllAlbums(JNIEnv* env, jobject obj, jint serverId, jint limit);
JNIEXPORT void JNICALL Java_com_y_player_JniInterface_getAllSongs(JNIEnv* env, jobject obj, jint serverId, jint limit);
JNIEXPORT void JNICALL Java_com_y_player_JniInterface_getAllPlaylists(JNIEnv* env, jobject obj, jint serverId, jint limit);
JNIEXPORT void JNICALL Java_com_y_player_JniInterface_getAllVideos(JNIEnv* env, jobject obj, jint serverId, jint limit);
JNIEXPORT void JNICALL Java_com_y_player_JniInterface_getAllPictures(JNIEnv* env, jobject obj, jint serverId, jint limit);
JNIEXPORT void JNICALL Java_com_y_player_JniInterface_getAllMusicObjects(JNIEnv* env, jobject obj, jint objectId, jint startingIndex,jint limit);
JNIEXPORT void JNICALL Java_com_y_player_JniInterface_getAllVideoObjects(JNIEnv* env, jobject obj, jint objectId, jint startingIndex,jint limit);
JNIEXPORT void JNICALL Java_com_y_player_JniInterface_getAllPictureObjects(JNIEnv* env, jobject obj, jint objectId, jint startingIndex,jint limit);
JNIEXPORT jint JNICALL Java_com_y_player_JniInterface_getDuration(JNIEnv* env, jobject obj, jint objectId);
JNIEXPORT jint JNICALL Java_com_y_player_JniInterface_getNumOfChilds(JNIEnv* env, jobject obj, jint objectId);
JNIEXPORT void JNICALL Java_com_y_player_JniInterface_stopControlPoint(JNIEnv* env, jobject obj);

static JNINativeMethod methods[] = {
{"startControlPoint", "()V", (void*)Java_com_y_player_JniInterface_startControlPoint},
{"initNativeLog", "()V", (void*)Java_com_y_player_JniInterface_initNativeLog},
{"closeNativeLog", "()V", (void*)Java_com_y_player_JniInterface_closeNativeLog},
{"selectServer", "(I)I", (void*)Java_com_y_player_JniInterface_selectServer},
{"clearServerSelection", "()V", (void*)Java_com_y_player_JniInterface_clearServerSelection},
{"selectRenderer", "(I)I", (void*)Java_com_y_player_JniInterface_selectRenderer},
{"getAllAlbums", "(II)V", (void*)Java_com_y_player_JniInterface_getAllAlbums},
{"getAllSongs", "(II)V", (void*)Java_com_y_player_JniInterface_getAllSongs},
{"getAllPlaylists", "(II)V", (void*)Java_com_y_player_JniInterface_getAllPlaylists},
{"getAllVideos", "(II)V", (void*)Java_com_y_player_JniInterface_getAllVideos},
{"getAllPictures", "(II)V", (void*)Java_com_y_player_JniInterface_getAllPictures},
{"getAllMusicObjects", "(III)V", (void*)Java_com_y_player_JniInterface_getAllMusicObjects},
{"getAllVideoObjects", "(III)V", (void*)Java_com_y_player_JniInterface_getAllVideoObjects},
{"getAllPictureObjects", "(III)V", (void*)Java_com_y_player_JniInterface_getAllPictureObjects},
{"getDuration", "(I)I", (void*)Java_com_y_player_JniInterface_getDuration},
{"getNumOfChilds", "(I)I", (void*)Java_com_y_player_JniInterface_getNumOfChilds},
{"stopControlPoint","()V", (void*)Java_com_y_player_JniInterface_stopControlPoint},
};

//void sayHelloWrapper();

static JNIEnv* getEnv() {
    JNIEnv * env = NULL;
    (*Jvm)->GetEnv(Jvm,(void **)&env, JNI_VERSION_1_4);
    if (!env) {
        (*Jvm)->AttachCurrentThread (Jvm,&env,NULL);
    }
    return env;
}

void detachThreadFromJVM (void) {
    (*Jvm)->DetachCurrentThread (Jvm);
}

jint JNI_OnLoad (JavaVM * vm, void * reserved) {
    Jvm = vm;
    JNIEnv * env;
    jclass cls;

    if ((*Jvm)->GetEnv(Jvm,(void **)&env, JNI_VERSION_1_4) != JNI_OK) {
	LOGEE("jni_interface:JNI_OnLoad Failed to get env. Hence Exiting");
        return -1;
    }

    cls = (*env)->FindClass(env,gJniInterfaceClsName);

    if (cls == NULL) {
	LOGEE("jni_interface:JNI_OnLoad Faild to Class %s",gJniInterfaceClsName);
        return -1;
    }
    if ((*env)->RegisterNatives(env,cls, methods, sizeof (methods)/ sizeof (methods[0])) < 0) {
	LOGEE("jni_interface:JNI_OnLoad Faild to Register Natives for the class %s",gJniInterfaceClsName);
        return -1;
    }

    cbNewServerAdded = (*env)->GetMethodID (env,cls, "NewServerAdded","(Ljava/lang/String;I)V");
    cbNewRendererAdded = (*env)->GetMethodID (env,cls, "NewRendererAdded","(Ljava/lang/String;I)V");
    cbNewServerRemoved = (*env)->GetMethodID (env,cls, "NewServerRemoved","(I)V");
    cbNewRendererRemoved = (*env)->GetMethodID (env,cls, "NewRendererRemoved","(I)V");
    cbNotifyGetAllSongs = (*env)->GetMethodID (env,cls, "NotifyGetAllSongs","([Ljava/lang/String;[I[Ljava/lang/String;)V");
    cbNotifyGetAllVideos = (*env)->GetMethodID (env,cls, "NotifyGetAllVideos","([Ljava/lang/String;[I[Ljava/lang/String;)V");
    cbNotifyGetAllPictures = (*env)->GetMethodID (env,cls, "NotifyGetAllPictures","([Ljava/lang/String;[I[Ljava/lang/String;)V");
    cbNotifyGetMusicObjects = (*env)->GetMethodID (env,cls, "NotifyGetMusicObjects","([Ljava/lang/String;[I[Ljava/lang/String;[Ljava/lang/String;)V");
    cbNotifyGetVideoObjects = (*env)->GetMethodID (env,cls, "NotifyGetVideoObjects","([Ljava/lang/String;[I[Ljava/lang/String;[Ljava/lang/String;)V");
    cbNotifyGetPictureObjects = (*env)->GetMethodID (env,cls, "NotifyGetPictureObjects","([Ljava/lang/String;[I[Ljava/lang/String;[Ljava/lang/String;)V");
    
    if (NULL == cbNewServerAdded || NULL == cbNewRendererAdded ||
    		NULL == cbNotifyGetAllSongs || NULL == cbNotifyGetAllVideos ||
    		NULL == cbNotifyGetAllPictures || NULL == cbNotifyGetMusicObjects ||
    		NULL == cbNotifyGetVideoObjects || NULL == cbNotifyGetPictureObjects) {

    	LOGEE("jni_interface:JNI_OnLoad Failed to get one of the Methods");
        return -1;
    }

    InitNativeLog();
    return JNI_VERSION_1_4;
}

/* JNI Mehtods which can be called from Java.. */
void
Java_com_y_player_JniInterface_initNativeLog(JNIEnv* env, jobject obj)
{
    LOGDD("jni_interface:initNativeLog IN");
    InitNativeLog();
}

void
Java_com_y_player_JniInterface_closeNativeLog(JNIEnv* env, jobject obj)
{
    LOGDD("jni_interface:closeNativeLog IN");
    CloseNativeLog();
}

void
Java_com_y_player_JniInterface_startControlPoint(JNIEnv* env, jobject obj)
{
    LOGDD("jni_interface:startControlPoint: calling startControlPoint");
    gJniInterfaceObj = (*env)->NewGlobalRef(env,obj);
    startControlPoint();
}

jint
Java_com_y_player_JniInterface_selectServer(JNIEnv* env, jobject obj, jint serverId)
{
    LOGDD("jni_interface:selectServer: calling selectServer with serverid = %d",serverId);
    return selectServer(serverId);
}

void
Java_com_y_player_JniInterface_clearServerSelection(JNIEnv* env, jobject obj)
{
    LOGDD("jni_interface:selectServer: calling clearServerSelection");
    clearServerSelection();
}

jint
Java_com_y_player_JniInterface_selectRenderer(JNIEnv* env, jobject obj, jint rendererId)
{
    LOGDD("jni_interface:selectRenderer: calling selectRenderer with rendererid = %d",rendererId);
    return selectRenderer(rendererId);
}

void
Java_com_y_player_JniInterface_getAllAlbums(JNIEnv* env, jobject obj, jint serverId, jint limit)
{
    LOGDD("jni_interface:getAllAlbums: calling getAllAlbums with serverid = %d and limit = %d",serverId,limit);
    getAllAlbums(serverId,limit);
}

void
Java_com_y_player_JniInterface_getAllSongs(JNIEnv* env, jobject obj, jint serverId, jint limit)
{
    LOGDD("jni_interface:getAllSongs: calling getAllSongs with serverid = %d and limit = %d",serverId,limit);
    getAllSongs(serverId,limit);
}

void
Java_com_y_player_JniInterface_getAllPlaylists(JNIEnv* env, jobject obj, jint serverId, jint limit)
{
    LOGDD("jni_interface:getAllPlaylists: calling getAllPlaylists with serverid = %d and limit = %d",serverId,limit);
     getAllPlaylists(serverId,limit);
}

void
Java_com_y_player_JniInterface_getAllVideos(JNIEnv* env, jobject obj, jint serverId, jint limit)
{
    LOGDD("jni_interface:getAllVideos: calling getAllVideos with serverid = %d and limit = %d",serverId,limit);
    getAllVideos(serverId,limit);
}

void
Java_com_y_player_JniInterface_getAllPictures(JNIEnv* env, jobject obj, jint serverId, jint limit)
{
    LOGDD("jni_interface:getAllPictures: calling getAllPictures with serverid = %d and limit = %d",serverId,limit);
    getAllPictures(serverId,limit);
}

void
Java_com_y_player_JniInterface_getAllMusicObjects(JNIEnv* env, jobject obj, jint objectId,jint startingIndex, jint limit)
{
    LOGDD("jni_interface:getAllMusicObjects: calling getAllMusicObjects with object ID = %d and limit = %d",objectId,limit);
    getAllMusicObjects(objectId,startingIndex,limit);
}

void
Java_com_y_player_JniInterface_getAllVideoObjects(JNIEnv* env, jobject obj, jint objectId, jint startIndex,jint limit)
{
    LOGDD("jni_interface:getAllVideoObjects: calling getAllVideoObjects with object ID = %d and limit = %d",objectId,limit);
    getAllVideoObjects(objectId,startIndex,limit);
}

void
Java_com_y_player_JniInterface_getAllPictureObjects(JNIEnv* env, jobject obj, jint objectId,jint startIndex, jint limit)
{
    LOGDD("jni_interface:getAllPictureObjects: calling getAllPictureObjects with object ID = %d and limit = %d",objectId,limit);
    getAllPictureObjects(objectId,startIndex,limit);
}

jint
Java_com_y_player_JniInterface_getDuration(JNIEnv* env, jobject obj, jint objectId)
{
    LOGDD("jni_interface:getDuration: calling getDuration with object ID = %d",objectId);
    return getDuration(objectId);
}

jint
Java_com_y_player_JniInterface_getNumOfChilds(JNIEnv* env, jobject obj, jint objectId)
{
    LOGDD("jni_interface:getDuration: calling getNumOfChilds with object ID = %d",objectId);
    return getNumOfChilds(objectId);
}

void
Java_com_y_player_JniInterface_stopControlPoint(JNIEnv* env, jobject obj)
{
    LOGDD("jni_interface:stopControlPoint: calling stopControlPoint");
    stopControlPoint();
}

/* Java Call back functions. */
void jniSongListCallback(char **songNames, int *songIds, char **songUrls, int count)
{
    JNIEnv *env = getEnv();

    jobjectArray SongNames;
    jobjectArray SongUrls;
    jintArray SongIds;
    int i;
    char *tempPtr = NULL;

    SongNames = (jobjectArray)(*env)->NewObjectArray(env,count,(*env)->FindClass(env,"java/lang/String"),(*env)->NewStringUTF(env,""));
    SongUrls = (jobjectArray)(*env)->NewObjectArray(env,count,(*env)->FindClass(env,"java/lang/String"),(*env)->NewStringUTF(env,""));
    SongIds = (*env)->NewIntArray(env, count);

    for(i=0;i<count;i++) {  
    	tempPtr = (char*)(songNames + i*64);
        (*env)->SetObjectArrayElement(env,SongNames,i,(*env)->NewStringUTF(env,tempPtr));
        tempPtr = (char*)(songUrls + i*256);
        (*env)->SetObjectArrayElement(env,SongUrls,i,(*env)->NewStringUTF(env,tempPtr));
    }
    (*env)->SetIntArrayRegion(env, SongIds, 0, count, songIds);

    if(songNames){
    	free(songNames);
	}
	if(songIds){
		free(songIds);
	}
	if(songUrls){
		free(songUrls);
	}
    LOGDD("jni_interface:jniSongListCallback: calling Java Callback Function with cbNotifyGetAllSongs");
    (*env)->CallVoidMethod(env,gJniInterfaceObj, cbNotifyGetAllSongs, SongNames, SongIds, SongUrls);
}

void jniVideoListCallback(char **videoNames, int *videoIds, char **videoUrls, int count)
{
    JNIEnv *env = getEnv();

    jobjectArray VideoNames;
    jobjectArray VideoUrls;
    jintArray VideoIds;
    int i;
    char *tempPtr = NULL;

    VideoNames = (jobjectArray)(*env)->NewObjectArray(env,count,(*env)->FindClass(env,"java/lang/String"),(*env)->NewStringUTF(env,""));
    VideoUrls = (jobjectArray)(*env)->NewObjectArray(env,count,(*env)->FindClass(env,"java/lang/String"),(*env)->NewStringUTF(env,""));
    VideoIds = (*env)->NewIntArray(env, count);

    for(i=0;i<count;i++) {  
    	tempPtr = (char*)(videoNames + i*64);
        (*env)->SetObjectArrayElement(env,VideoNames,i,(*env)->NewStringUTF(env,tempPtr));
        tempPtr = (char*)(videoUrls + i*256);
        (*env)->SetObjectArrayElement(env,VideoUrls,i,(*env)->NewStringUTF(env,tempPtr));
    }
    (*env)->SetIntArrayRegion(env, VideoIds, 0, count, videoIds);

    if(videoNames){
    	free(videoNames);
	}
	if(videoIds){
		free(videoIds);
	}
	if(videoUrls){
		free(videoUrls);
	}
    LOGDD("jni_interface:jniVideoListCallback: calling Java Callback Function with cbNotifyGetAllVideos");
    (*env)->CallVoidMethod(env,gJniInterfaceObj, cbNotifyGetAllVideos, VideoNames, VideoIds, VideoUrls);
}

void jniPictureListCallback(char **pictureNames, int *pictureIds, char **pictureUrls, int count)
{
    JNIEnv *env = getEnv();

    jobjectArray PictureNames;
    jobjectArray PictureUrls;
    jintArray PictureIds;
    int i;
    char *tempPtr = NULL;

    PictureNames = (jobjectArray)(*env)->NewObjectArray(env,count,(*env)->FindClass(env,"java/lang/String"),(*env)->NewStringUTF(env,""));
    PictureUrls = (jobjectArray)(*env)->NewObjectArray(env,count,(*env)->FindClass(env,"java/lang/String"),(*env)->NewStringUTF(env,""));
    PictureIds = (*env)->NewIntArray(env, count);

    for(i=0;i<count;i++) {  
    	tempPtr = (char*)(pictureNames + i*64);
        (*env)->SetObjectArrayElement(env,PictureNames,i,(*env)->NewStringUTF(env,tempPtr));
        tempPtr = (char*)(pictureUrls + i*256);
        (*env)->SetObjectArrayElement(env,PictureUrls,i,(*env)->NewStringUTF(env,tempPtr));
    }
    (*env)->SetIntArrayRegion(env, PictureIds, 0, count, pictureIds);
    if(pictureNames){
    	free(pictureNames);
	}
	if(pictureIds){
		free(pictureIds);
	}
	if(pictureUrls){
		free(pictureUrls);
	}
    LOGDD("jni_interface:jniPictureListCallback: calling Java Callback Function with cbNotifyGetAllPictures");
    (*env)->CallVoidMethod(env,gJniInterfaceObj, cbNotifyGetAllPictures, PictureNames, PictureIds, PictureUrls);
}

void jniMusicObjectCallback(char **objectNames, int *objectIds, char **objectUrls,char **thumbUrls, int count)
{
    JNIEnv *env = getEnv();

    jobjectArray ObjectNames;
    jobjectArray ObjectUrls;
    jobjectArray ThumbUrls;
    jintArray ObjectIds;
    int i;
    char *tempPtr = NULL;

    ObjectNames = (jobjectArray)(*env)->NewObjectArray(env,count,(*env)->FindClass(env,"java/lang/String"),(*env)->NewStringUTF(env,""));
    ObjectUrls = (jobjectArray)(*env)->NewObjectArray(env,count,(*env)->FindClass(env,"java/lang/String"),(*env)->NewStringUTF(env,""));
    ThumbUrls = (jobjectArray)(*env)->NewObjectArray(env,count,(*env)->FindClass(env,"java/lang/String"),(*env)->NewStringUTF(env,""));
    ObjectIds = (*env)->NewIntArray(env, count);

    for(i=0;i<count;i++) {
    	tempPtr = (char*)objectNames + i*64;
    	if(strlen(tempPtr)){
    		(*env)->SetObjectArrayElement(env,ObjectNames,i,(*env)->NewStringUTF(env,tempPtr));
    		LOGDD("jni_interface:jniMusicObjectCallback: Copied the Object with Name = %s and Object ID = %d",tempPtr,objectIds[i]);
    	}
        tempPtr = (char*)objectUrls + i*256;
        if(strlen(tempPtr)){
        	(*env)->SetObjectArrayElement(env,ObjectUrls,i,(*env)->NewStringUTF(env,tempPtr));
        	LOGDD("jni_interface:jniMusicObjectCallback: Copied the Object with URL = %s",tempPtr);
        }
        tempPtr = (char*)thumbUrls + i*256;
		if(strlen(tempPtr)){
			(*env)->SetObjectArrayElement(env,ThumbUrls,i,(*env)->NewStringUTF(env,tempPtr));
			LOGDD("jni_interface:jniMusicObjectCallback: Copied the Object with Thumbnail URL = %s",tempPtr);
		}
    }
    (*env)->SetIntArrayRegion(env, ObjectIds, 0, count, objectIds);
    if(objectNames){
    	free(objectNames);
    }
    if(objectIds){
    	free(objectIds);
    }
    if(objectUrls){
    	free(objectUrls);
    }
    if(thumbUrls){
		free(thumbUrls);
	}
    LOGDD("jni_interface:jniMusicObjectCallback: calling Java Callback Function with cbNotifyGetMusicObjects");
    (*env)->CallVoidMethod(env,gJniInterfaceObj, cbNotifyGetMusicObjects, ObjectNames, ObjectIds, ObjectUrls,ThumbUrls);
}

void jniVideoObjectCallback(char **objectNames, int *objectIds, char **objectUrls,char **thumbUrls, int count)
{
	JNIEnv *env = getEnv();

	    jobjectArray ObjectNames;
	    jobjectArray ObjectUrls;
	    jobjectArray ThumbUrls;
	    jintArray ObjectIds;
	    int i;
	    char *tempPtr = NULL;

	    ObjectNames = (jobjectArray)(*env)->NewObjectArray(env,count,(*env)->FindClass(env,"java/lang/String"),(*env)->NewStringUTF(env,""));
	    ObjectUrls = (jobjectArray)(*env)->NewObjectArray(env,count,(*env)->FindClass(env,"java/lang/String"),(*env)->NewStringUTF(env,""));
	    ThumbUrls = (jobjectArray)(*env)->NewObjectArray(env,count,(*env)->FindClass(env,"java/lang/String"),(*env)->NewStringUTF(env,""));
	    ObjectIds = (*env)->NewIntArray(env, count);

	    for(i=0;i<count;i++) {
	    	tempPtr = (char*)objectNames + i*64;
			if(strlen(tempPtr)){
				(*env)->SetObjectArrayElement(env,ObjectNames,i,(*env)->NewStringUTF(env,tempPtr));
				LOGDD("jni_interface:jniVideoObjectCallback: Copied the Object with Name = %s",tempPtr);
			}

			tempPtr = (char*)objectUrls + i*256;
			if(strlen(tempPtr)){
				(*env)->SetObjectArrayElement(env,ObjectUrls,i,(*env)->NewStringUTF(env,tempPtr));
				LOGDD("jni_interface:jniVideoObjectCallback: Copied the Object with URL = %s",tempPtr);
			}
			tempPtr = (char*)thumbUrls + i*256;
			if(strlen(tempPtr)){
				(*env)->SetObjectArrayElement(env,ThumbUrls,i,(*env)->NewStringUTF(env,tempPtr));
				LOGDD("jni_interface:jniVideoObjectCallback: Copied the Object with Thumbnail URL = %s",tempPtr);
			}
	    }
	    (*env)->SetIntArrayRegion(env, ObjectIds, 0, count, objectIds);
	    if(objectNames){
	    	free(objectNames);
	    }
	    if(objectIds){
	    	free(objectIds);
	    }
	    if(objectUrls){
	    	free(objectUrls);
	    }
	    if(thumbUrls){
			free(thumbUrls);
		}
	    LOGDD("jni_interface:jniVideoObjectCallback: calling Java Callback Function with cbNotifyGetMusicObjects");
	    (*env)->CallVoidMethod(env,gJniInterfaceObj, cbNotifyGetVideoObjects, ObjectNames, ObjectIds, ObjectUrls,ThumbUrls);
}

void jniPictureObjectCallback(char **objectNames, int *objectIds, char **objectUrls,char **thumbUrls, int count)
{
	JNIEnv *env = getEnv();

	    jobjectArray ObjectNames;
	    jobjectArray ObjectUrls;
	    jobjectArray ThumbUrls;
	    jintArray ObjectIds;
	    int i;
	    char *tempPtr = NULL;

	    ObjectNames = (jobjectArray)(*env)->NewObjectArray(env,count,(*env)->FindClass(env,"java/lang/String"),(*env)->NewStringUTF(env,""));
	    ObjectUrls = (jobjectArray)(*env)->NewObjectArray(env,count,(*env)->FindClass(env,"java/lang/String"),(*env)->NewStringUTF(env,""));
	    ThumbUrls = (jobjectArray)(*env)->NewObjectArray(env,count,(*env)->FindClass(env,"java/lang/String"),(*env)->NewStringUTF(env,""));
	    ObjectIds = (*env)->NewIntArray(env, count);

	    for(i=0;i<count;i++) {
	    	tempPtr = (char*)objectNames + i*64;
			if(strlen(tempPtr)){
				(*env)->SetObjectArrayElement(env,ObjectNames,i,(*env)->NewStringUTF(env,tempPtr));
				LOGDD("jni_interface:jniPictureListCallback: Copied the Object with Name = %s",tempPtr);
			}

			tempPtr = (char*)objectUrls + i*256;
			if(strlen(tempPtr)){
				(*env)->SetObjectArrayElement(env,ObjectUrls,i,(*env)->NewStringUTF(env,tempPtr));
				LOGDD("jni_interface:jniPictureListCallback: Copied the Object with URL = %s",tempPtr);
			}
			tempPtr = (char*)thumbUrls + i*256;
			if(strlen(tempPtr)){
				(*env)->SetObjectArrayElement(env,ThumbUrls,i,(*env)->NewStringUTF(env,tempPtr));
				LOGDD("jni_interface:jniPictureListCallback: Copied the Object with Thumbnail URL = %s",tempPtr);
			}
	    }
	    (*env)->SetIntArrayRegion(env, ObjectIds, 0, count, objectIds);
	    if(objectNames){
	    	free(objectNames);
	    }
	    if(objectIds){
	    	free(objectIds);
	    }
	    if(objectUrls){
	    	free(objectUrls);
	    }
	    if(thumbUrls){
			free(thumbUrls);
		}
	    LOGDD("jni_interface:jniPictureListCallback: calling Java Callback Function with cbNotifyGetMusicObjects");
	    (*env)->CallVoidMethod(env,gJniInterfaceObj, cbNotifyGetPictureObjects, ObjectNames, ObjectIds, ObjectUrls,ThumbUrls);
}

void notifyServerAdded(char *serverName, int serverId)
{
    JNIEnv *env = getEnv(); 
    jstring server = (*env)->NewStringUTF(env,serverName);
    jint serverID = serverId;
    LOGDD("jni_interface:notifyServerAdded: calling Java Callback Function with cbNewServerAdded");
    (*env)->CallVoidMethod(env,gJniInterfaceObj, cbNewServerAdded, server, serverID);
    detachThreadFromJVM();
}

void notifyRendererAdded(char *rendererName, int rendererId)
{
    JNIEnv *env = getEnv();
    jstring renderer = (*env)->NewStringUTF(env,rendererName);
    jint rendererID = rendererId;
    LOGDD("jni_interface:notifyRendererAdded: calling Java Callback Function with cbNewRendererAdded");
    (*env)->CallVoidMethod(env,gJniInterfaceObj, cbNewRendererAdded, renderer, rendererID);
    detachThreadFromJVM();
}

void notifyServerRemoved(int serverId)
{
    JNIEnv *env = getEnv(); 
    jint serverID = serverId;
    LOGDD("jni_interface:notifyServerRemoved: calling Java Callback Function with cbNewServerRemoved");
    (*env)->CallVoidMethod(env,gJniInterfaceObj, cbNewServerRemoved, serverID);
    detachThreadFromJVM();
}

void notifyRendererRemoved(int rendererId)
{
    JNIEnv *env = getEnv();
    jint rendererID = rendererId;
    LOGDD("jni_interface:notifyRendererRemoved: calling Java Callback Function with cbNewServerRemoved");
    (*env)->CallVoidMethod(env,gJniInterfaceObj, cbNewServerRemoved, rendererID);
    detachThreadFromJVM();
}

#include "common/upnp/upnp_controlpoint_wrapper.h"
#include "common/upnp/upnp_control_point.h"
#include "common/util/linkedlist.h"
#include "common/log/log.h"

static UPNPControlPointWrapper gControlPoint;

extern "C" {
	void jniSongListCallback(char **songNames, int *songIds, char **songUrls, int count);
	void jniVideoListCallback(char **videoNames, int *videoIds, char **videoUrls, int count);
	void jniPictureListCallback(char **pictureNames, int *pictureIds, char **pictureUrls, int count);
	void jniMusicObjectCallback(char **objectNames, int *objectIds, char **objectUrls,char **thumbUrls, int count);
	void jniVideoObjectCallback(char **objectNames, int *objectIds, char **objectUrls,char **thumbUrls, int count);
	void jniPictureObjectCallback(char **objectNames, int *objectIds, char **objectUrls,char **thumbUrls, int count);
	void notifyServerAdded(char *serverName, int serverId);
	void notifyRendererAdded(char *rendererName, int rendererId);
	void notifyServerRemoved(int serverId);

	void startControlPoint() 
	{
	    LOGD("control_point:startControlPoint IN");
	    gControlPoint.startUPNPControlPoint();
	}


	int selectServer(int serverID)
	{
	    LOGD("control_point:selectServer serverId = %d",serverID);
	    return gControlPoint.SelectServer(serverID);
	}

	void clearServerSelection()
		{
		    LOGD("control_point:clearServerSelection IN");
		    gControlPoint.ClearServerSelection();
		}

	int selectRenderer(int serverID)
	{
	    LOGD("control_point:selectRenderer serverId = %d",serverID);
	    return gControlPoint.SelectRenderer(serverID);
	}

	void getAllAlbums(int serverId, int limit)
	{
	    LOGD("control_point:getAllAlbums serverId = %d limit = %d",serverId,limit);
	    return;
	}

	void getAllSongs(int serverId, int limit)
	{
	    LOGD("control_point:getAllSongs serverId = %d limit = %d",serverId,limit);
	    cLinkedList<MediaObj> *mediaObj = gControlPoint.GetAllSongs(serverId);
	    int count = mediaObj->GetNumberOfElements(), i=0;
	    if(count <= 0){
	    	return;
	    }
	    char **songNames = (char**)os_malloc(count * sizeof(char) * 64); //Assuming max song name length is 64.
	    int *songIds = (int*)os_malloc(count * sizeof(int));
	    char **urlNames = (char**)os_malloc(count * sizeof(char) * 256); //Assuming max url length of each song is 256.

	    if(NULL == songNames || NULL == songIds || NULL == urlNames){
			LOGD("control_point:getAllPictures Failure to allocate the Memory");
			return;
		}else{
			os_memset(songNames,'\0',(count * sizeof(char) * 64));
			os_memset(songIds,'\0',count*sizeof(int));
			os_memset(urlNames,'\0',(count * sizeof(char) * 256));
		}

	    for(i=1;i<=count;i++){
			MediaObj tempObj;
			char *tempPtr = NULL;
			os_memset(&tempObj,0,sizeof(tempObj));
			if(SUCCESS != mediaObj->GetElementAtIndex(tempObj,i)){
				 break;
			}
			LOGD("control_point:getAllSongs Retrieved the Object with Name = %s ID = %d URL = %s",tempObj.Title,tempObj.ID,tempObj.URL);
			if(os_strlen(tempObj.Title) != 0){
				tempPtr = (char*)(songNames + ((i-1) * 64));
				 os_strncpy(tempPtr,tempObj.Title,64);
				 LOGD("control_point:getAllSongs adding the song [%s] to the list",tempPtr);
			}
			if(os_strlen(tempObj.ID) != 0){
				 songIds[i-1] = os_atoi(tempObj.ID);
			}
			if(os_strlen(tempObj.URL) != 0){
				tempPtr = (char*)(urlNames + ((i-1) * 256));
				 os_strncpy(tempPtr,tempObj.URL,256);
				 LOGD("control_point:getAllSongs adding the song URL [%s] to the list",tempPtr);
			}
	    }

	    jniSongListCallback(songNames,songIds,urlNames,count);
	    return;
	}

	void getAllPlaylists(int serverId, int limit)
	{
	    return;
	}

	void getAllVideos(int serverId, int limit)
	{
	    cLinkedList<MediaObj> *mediaObj = gControlPoint.GetAllVideos(serverId);
	    int count = mediaObj->GetNumberOfElements(), i=0;
	    if(count <= 0){
	    	return;
	    }
	    char **videoNames = (char**)os_malloc(count * sizeof(char) * 64); //Assuming max video name length is 64.
	    int *videoIds = (int*)os_malloc(count * sizeof(int));
	    char **urlNames = (char**)os_malloc(count * sizeof(char) * 256); //Assuming max url length of each video is 256.

	    if(NULL == videoNames || NULL == videoIds || NULL == urlNames){
			LOGD("control_point:getAllPictures Failure to allocate the Memory");
			return;
		}else{
			os_memset(videoNames,'\0',(count * sizeof(char) * 64));
			os_memset(videoIds,'\0',count*sizeof(int));
			os_memset(urlNames,'\0',(count * sizeof(char) * 256));
		}

	    for(i=1;i<=count;i++){
			MediaObj tempObj;
			char *tempPtr = NULL;
			os_memset(&tempObj,0,sizeof(tempObj));
			if(SUCCESS != mediaObj->GetElementAtIndex(tempObj,i)){
				 break;
			}
			LOGD("control_point:getAllVideos Retrieved the Object with Name = %s ID = %d URL = %s",tempObj.Title,tempObj.ID,tempObj.URL);
			if(os_strlen(tempObj.Title) != 0){
				tempPtr = (char*)(videoNames + ((i-1) * 64));
				 os_strncpy(tempPtr,tempObj.Title,64);
				 LOGD("control_point:getAllVideos adding the song [%s] to the list",tempPtr);
			}
			if(os_strlen(tempObj.ID) != 0){
				 videoIds[i-1] = os_atoi(tempObj.ID);
			}
			if(os_strlen(tempObj.URL) != 0){
				tempPtr = (char*)(urlNames + ((i-1) * 256));
				 os_strncpy(tempPtr,tempObj.URL,256);
				 LOGD("control_point:getAllVideos adding the song URL [%s] to the list",tempPtr);
			}
	    }

	    jniVideoListCallback(videoNames,videoIds,urlNames,count);
	    return;
	}

	void getAllPictures(int serverId, int limit)
	{
	    cLinkedList<MediaObj> *mediaObj = gControlPoint.GetAllPictures(serverId);
	    int count = mediaObj->GetNumberOfElements(), i=0;
	    if(count <= 0){
	    	return;
	    }
	    char **pictureNames = (char**)os_malloc(count * sizeof(char) * 64); //Assuming max picture name length is 64.
	    int *pictureIds = (int*)os_malloc(count * sizeof(int));
	    char **urlNames = (char**)os_malloc(count * sizeof(char) * 256); //Assuming max url length of each picture is 256.

	    if(NULL == pictureNames || NULL == pictureIds || NULL == urlNames){
			LOGD("control_point:getAllPictures Failure to allocate the Memory");
			return;
		}else{
			os_memset(pictureNames,'\0',(count * sizeof(char) * 64));
			os_memset(pictureIds,'\0',count*sizeof(int));
			os_memset(urlNames,'\0',(count * sizeof(char) * 256));
		}

	    for(i=1;i<=count;i++){
			MediaObj tempObj;
			char *tempPtr = NULL;
			os_memset(&tempObj,0,sizeof(tempObj));
			if(SUCCESS != mediaObj->GetElementAtIndex(tempObj,i)){
				 break;
			}
			LOGD("control_point:getAllPictures Retrieved the Object with Name = %s ID = %d URL = %s",tempObj.Title,tempObj.ID,tempObj.URL);
			if(os_strlen(tempObj.Title) != 0){
				tempPtr = (char*)(pictureNames + ((i-1) * 64));
				 os_strncpy(tempPtr,tempObj.Title,64);
				 LOGD("control_point:getAllPictures adding the song [%s] to the list",tempPtr);
			}
			if(os_strlen(tempObj.ID) != 0){
				 pictureIds[i-1] = os_atoi(tempObj.ID);
			}
			if(os_strlen(tempObj.URL) != 0){
				tempPtr = (char*)(urlNames + ((i-1) * 256));
				 os_strncpy(tempPtr,tempObj.URL,256);
				 LOGD("control_point:getAllPictures adding the song URL [%s] to the list",tempPtr);
			}
	    }

	    jniPictureListCallback(pictureNames,pictureIds,urlNames,count);
	    return;
	}

	void getAllMusicObjects(int objectId, int startIndex,int limit)
	{
		LOGD("control_point:getAllMusicObjects IN ObjectID = %d, startIndex = %d, limit = %d",objectId,startIndex,limit);
		cLinkedList<MetaDataObj> mediaObj;
		LOGD("control_point:getAllMusicObjects calling gControlPoint.GetMusicObjects");
		gControlPoint.GetMusicObjects(objectId,mediaObj,startIndex,limit);
		LOGD("control_point:getAllMusicObjects returned from gControlPoint.GetMusicObjects with Count = %d",mediaObj.GetNumberOfElements());
		int count = mediaObj.GetNumberOfElements(), i=0;
		if(count <= 0){
			jniMusicObjectCallback(NULL,NULL,NULL,NULL,count);
			return;
		}
		char **objectNames = (char**)os_malloc(count * (sizeof(char) * 64)); //Assuming max picture name length is 64.
		int *objectIds = (int*)os_malloc(count * sizeof(int));
		char **urlNames = (char**)os_malloc(count * (sizeof(char) * 256)); //Assuming max url length of each picture is 256.
		char **thumbUrls = (char**)os_malloc(count * (sizeof(char) * 256)); //Assuming max url length of each picture is 256.

		if(NULL == objectNames || NULL == objectIds || NULL == urlNames || NULL == thumbUrls){
			LOGD("control_point:getAllMusicObjects Failure to allocate the Memory");
			jniMusicObjectCallback(objectNames,objectIds,urlNames,thumbUrls,0);
			return;
		}else{
			os_memset(objectNames,'\0',(count * sizeof(char) * 64));
			os_memset(objectIds,'\0',count*sizeof(int));
			os_memset(urlNames,'\0',(count * sizeof(char) * 256));
			os_memset(thumbUrls,'\0',(count * sizeof(char) * 256));
		}

		for(i=1;i<=count;i++){
			MetaDataObj tempObj;
			char *tempPtr = NULL;
			os_memset(&tempObj,0,sizeof(tempObj));
			if(SUCCESS != mediaObj.GetElementAtIndex(tempObj,i)){
				 break;
			}
			LOGD("control_point:getAllMusicObjects Retrieved the Object with Name = %s ID = %d URL = %s",tempObj.Title,tempObj.ObjectID,tempObj.URL);
			if(os_strlen(tempObj.Title) != 0){
				tempPtr = (char*)objectNames + ((i-1) * 64);
				 os_strncpy(tempPtr,tempObj.Title,64);
				 LOGD("control_point:getAllMusicObjects adding the song [%s] to the list",tempPtr);
			}

			objectIds[i-1] = tempObj.ObjectID;
			LOGD("control_point:getAllMusicObjects adding the song ID[%d] to the list",objectIds[i-1]);

			if(os_strlen(tempObj.URL) != 0 && !tempObj.Container){
				tempPtr = (char*)urlNames + ((i-1) * 256);
				 os_strncpy(tempPtr,tempObj.URL,256);
				 LOGD("control_point:getAllMusicObjects adding the song URL [%s] to the list",tempPtr);
			}
			if(os_strlen(tempObj.ThumbnailURL) != 0){
				tempPtr = (char*)thumbUrls + ((i-1) * 256);
				 os_strncpy(tempPtr,tempObj.ThumbnailURL,256);
				 LOGD("control_point:getAllMusicObjects adding the thumbnail URL [%s] to the list",tempPtr);
			}
		}

		jniMusicObjectCallback(objectNames,objectIds,urlNames,thumbUrls,count);
		return;
	}

	void getAllVideoObjects(int objectId, int startIndex,int limit)
	{
		LOGD("control_point:getAllVideoObjects IN ObjectID = %d, startIndex = %d, limit = %d",objectId,startIndex,limit);
		cLinkedList<MetaDataObj> mediaObj;
		LOGD("control_point:getAllVideoObjects calling gControlPoint.GetVideoObjects");
		gControlPoint.GetVideoObjects(objectId,mediaObj,startIndex,limit);
		LOGD("control_point:getAllVideoObjects returned from gControlPoint.GetVideoObjects with Count = %d",mediaObj.GetNumberOfElements());
		int count = mediaObj.GetNumberOfElements(), i=0;
		if(count <= 0){
			jniVideoObjectCallback(NULL,NULL,NULL,NULL,count);
			return;
		}
		char **objectNames = (char**)os_malloc(count * (sizeof(char) * 64)); //Assuming max picture name length is 64.
		int *objectIds = (int*)os_malloc(count * sizeof(int));
		char **urlNames = (char**)os_malloc(count * (sizeof(char) * 256)); //Assuming max url length of each picture is 256.
		char **thumbUrls = (char**)os_malloc(count * (sizeof(char) * 256)); //Assuming max url length of each picture is 256.

		if(NULL == objectNames || NULL == objectIds || NULL == urlNames || NULL == thumbUrls){
			LOGD("control_point:getAllMusicObjects Failure to allocate the Memory");
			jniVideoObjectCallback(objectNames,objectIds,urlNames,thumbUrls,0);
			return;
		}else{
			os_memset(objectNames,'\0',(count * sizeof(char) * 64));
			os_memset(objectIds,'\0',count*sizeof(int));
			os_memset(urlNames,'\0',(count * sizeof(char) * 256));
			os_memset(thumbUrls,'\0',(count * sizeof(char) * 256));
		}

		for(i=1;i<=count;i++){
			MetaDataObj tempObj;
			char *tempPtr = NULL;
			os_memset(&tempObj,0,sizeof(tempObj));
			if(SUCCESS != mediaObj.GetElementAtIndex(tempObj,i)){
				 break;
			}
			LOGD("control_point:getAllVideoObjects Retrieved the Object with Name = %s ID = %d URL = %s",tempObj.Title,tempObj.ObjectID,tempObj.URL);
			if(os_strlen(tempObj.Title) != 0){
				tempPtr = (char*)objectNames + ((i-1) * 64);
				 os_strncpy(tempPtr,tempObj.Title,64);
				 LOGD("control_point:getAllVideoObjects adding the song [%s] to the list",tempPtr);
			}

			objectIds[i-1] = tempObj.ObjectID;
			LOGD("control_point:getAllVideoObjects adding the song ID[%d] to the list",objectIds[i-1]);

			if(os_strlen(tempObj.URL) != 0 && !tempObj.Container){
				tempPtr = (char*)urlNames + ((i-1) * 256);
				 os_strncpy(tempPtr,tempObj.URL,256);
				 LOGD("control_point:getAllVideoObjects adding the song URL [%s] to the list",tempPtr);
			}
			if(os_strlen(tempObj.ThumbnailURL) != 0){
				tempPtr = (char*)thumbUrls + ((i-1) * 256);
				 os_strncpy(tempPtr,tempObj.ThumbnailURL,256);
				 LOGD("control_point:getAllMusicObjects adding the thumbnail URL [%s] to the list",tempPtr);
			}
		}

		jniVideoObjectCallback(objectNames,objectIds,urlNames,thumbUrls,count);
		return;
	}

	void getAllPictureObjects(int objectId, int startIndex, int limit)
	{
		LOGD("control_point:getAllPictureObjects IN ObjectID = %d, startIndex = %d,limit = %d",objectId,startIndex,limit);
		cLinkedList<MetaDataObj> mediaObj;
		LOGD("control_point:getAllPictureObjects calling gControlPoint.GetPictureObjects");
		gControlPoint.GetPictureObjects(objectId,mediaObj,startIndex,limit);
		LOGD("control_point:getAllPictureObjects returned from gControlPoint.GetPictureObjects with Count = %d",mediaObj.GetNumberOfElements());
		int count = mediaObj.GetNumberOfElements(), i=0;
		if(count <= 0){
			jniPictureObjectCallback(NULL,NULL,NULL,NULL,count);
			return;
		}
		char **objectNames = (char**)os_malloc(count * (sizeof(char) * 64)); //Assuming max picture name length is 64.
		int *objectIds = (int*)os_malloc(count * sizeof(int));
		char **urlNames = (char**)os_malloc(count * (sizeof(char) * 256)); //Assuming max url length of each picture is 256.
		char **thumbUrls = (char**)os_malloc(count * (sizeof(char) * 256)); //Assuming max url length of each picture is 256.

		if(NULL == objectNames || NULL == objectIds || NULL == urlNames || NULL == thumbUrls){
			LOGD("control_point:getAllPictureObjects Failure to allocate the Memory");
			jniPictureObjectCallback(objectNames,objectIds,urlNames,thumbUrls,0);
			return;
		}else{
			os_memset(objectNames,'\0',(count * sizeof(char) * 64));
			os_memset(objectIds,'\0',count*sizeof(int));
			os_memset(urlNames,'\0',(count * sizeof(char) * 256));
			os_memset(thumbUrls,'\0',(count * sizeof(char) * 256));
		}

		for(i=1;i<=count;i++){
			MetaDataObj tempObj;
			char *tempPtr = NULL;
			os_memset(&tempObj,0,sizeof(tempObj));
			if(SUCCESS != mediaObj.GetElementAtIndex(tempObj,i)){
				 break;
			}
			LOGD("control_point:getAllPictureObjects Retrieved the Object with Name = %s ID = %d URL = %s",tempObj.Title,tempObj.ObjectID,tempObj.URL);
			if(os_strlen(tempObj.Title) != 0){
				tempPtr = (char*)objectNames + ((i-1) * 64);
				 os_strncpy(tempPtr,tempObj.Title,64);
				 LOGD("control_point:getAllPictureObjects adding the song [%s] to the list",tempPtr);
			}

			objectIds[i-1] = tempObj.ObjectID;
			LOGD("control_point:getAllPictureObjects adding the song ID[%d] to the list",objectIds[i-1]);

			if(os_strlen(tempObj.URL) != 0 && !tempObj.Container){
				tempPtr = (char*)urlNames + ((i-1) * 256);
				 os_strncpy(tempPtr,tempObj.URL,256);
				 LOGD("control_point:getAllPictureObjects adding the song URL [%s] to the list",tempPtr);
			}
			if(os_strlen(tempObj.ThumbnailURL) != 0){
				tempPtr = (char*)thumbUrls + ((i-1) * 256);
				 os_strncpy(tempPtr,tempObj.ThumbnailURL,256);
				 LOGD("control_point:getAllMusicObjects adding the thumbnail URL [%s] to the list",tempPtr);
			}
		}

		jniPictureObjectCallback(objectNames,objectIds,urlNames,thumbUrls,count);
		return;
	}

	int32 getDuration(int objectId)
	{
		return gControlPoint.GetDuration(objectId);
	}

	int32 getNumOfChilds(int objectId)
	{
		return gControlPoint.GetNumOfChilds(objectId);
	}

	void stopControlPoint(){
		gControlPoint.StopControlPoint();
	}
}

void notifyCPServerAdded(char *serverName, int serverId){
	LOGD("control_point:notifyCPServerAdded serverName = %s serverId = %d",serverName,serverId);
	notifyServerAdded(serverName,serverId);
}

void notifyCPServerRemoved(int serverId){
	LOGD("control_point:notifyCPServerRemoved serverId = %d",serverId);
	notifyServerRemoved(serverId);
}

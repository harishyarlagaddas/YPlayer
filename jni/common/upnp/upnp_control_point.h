
#ifndef UPNP_CONTROL_POINT_H_INCLUDED
#define UPNP_CONTROL_POINT_H_INCLUDED

#include "../include/basic_datatypes.h"
#include "../../os/os.h"
#include "../include/return_codes.h"
#include "../log/log.h"
#include "upnp_discovery.h"
#include "../util/XmlUtils.h"
#include "../../external/tinyxml/tinyxml.h"

#define UPNP_CP_CACHE_MAX_FOLDER_DEPTH 4
#define UPNP_CP_CACHE_MAX_ITEMS_IN_FOLDER 35

typedef enum{
	SERVER_ADDED = 1,
	RENDERER_ADDED,
	SERVER_REMOVED,
	RENDERER_REMOVED
}CPEventID;

typedef struct{
	CPEventID EventID;
	int32 DeviceID;
	int8 DeviceName[64];
	int32 ObjID;
	int8 ObjName[64];
	flag Container;
}CPObject;

typedef enum contentType{
	MUSIC = 1,
	VIDEO = 2,
	PICTURES = 4,
	MUSIC_VIDEO = 3,
	MUSIC_PICTURES = 5,
	VIDEO_PICTURES = 6,
	MUSIC_VIDEO_PICTURES = 7,
}ContentType;

typedef struct metaDataObj{
	int8 Title[64];
	int8 Creator[64];
	int8 Artist[64];
	int8 Album[64];
	int8 Genre[64];
	int32 Size;
	int32 Duration;
	int8 ProtocolInfo[128];
	int8 ThumbnailURL[128];
	int8 URL[128];
	flag Container;
	int32 ObjectID;
	int32 FolderDepth;
	int32 ChildCount;
	int8 ID[128];
	int8 ParentID[16];
	flag ObjectsAlreadyRetrieved;
	ContentType ContainItemsType;
	cLinkedList<struct metaDataObj> *ChildList;
	struct metaDataObj*ParentPtr;
}MetaDataObj;

typedef struct mediaObj{
	int8 Title[64];
	int8 Creator[64];
	int8 Artist[64];
	int8 Album[64];
	int8 Genre[64];
	int32 Size;
	int32 Duration;
	int8 ProtocolInfo[128];
	int8 ThumbnailURL[128];
	int8 URL[128];
	int8 ID[128];
	int32 DeviceID;
	int8 DeviceName[64];
}MediaObj;

class UPnPControlPointObserver
{
  public:
	UPnPControlPointObserver(){};
	virtual ~UPnPControlPointObserver(){};

	virtual void CPEventReceived(CPObject event) = 0;
};

class cUPnPControlPoint : public UPnPControlPointInterface, public cSocketObserver
{
public:
	cUPnPControlPoint();
	virtual ~cUPnPControlPoint();

	ReturnStatus StartControlPoint();
	void StopControlPoint();

	void SetSSDPInfo(const SSDPInfoStruct &aSSDPInfo);
	static void HandleSignal(int);

	void RegisterControlPointObserver(UPnPControlPointObserver *aObserver);
	/* Pure Virtual Functions from UPnPControlPointInterface */
	ReturnStatus AddServer(int8 *aUrl, int8* aUUID, int32 aLifeTime);
	ReturnStatus AddRenderer(int8 *aUrl, int8* aUUID, int32 aLifeTime);

	ReturnStatus RemoveServer(int8* aUUID);
	ReturnStatus RemoveRenderer(int8* aUUID);

	ReturnStatus SelectServer(int32 ServerID, flag aCacheData);
	void ClearServerSelection();
	cLinkedList<MediaObj>* GetAllSongs(int32 aServerID);
	cLinkedList<MediaObj>* GetAllVideos(int32 aServerID);
	cLinkedList<MediaObj>* GetAllPictures(int32 aServerID);
	ReturnStatus GetMetaData(int32 aServerID, int32 aObjectID, MetaDataObj &aMetadataObj);
	ReturnStatus GetObjects(int32 aObjectID, cLinkedList<MetaDataObj> &aObjList, int32 startIndex, int32 limit,ContentType type);
	int32 GetDuration(int32 aObjectID);
	int32 GetNumOfChilds(int32 aObjectID);

	/* Virtual Functions from cSocketObserver */
	void HandleSocketActivity(IN SocketObserverParams &aSockObsParms);
private:

	ReturnStatus ParseURL(int8 *aUrl, int8* aAddr, int32 &aPortNo, int8* aString);
	ReturnStatus GetDeviceInfo(int8 *aUrl, int8* aUUID, int32 aLifeTime,DeviceInfo *aDeviceInfo);
	ReturnStatus ParseDeviceInfo(int8 *aDeviceDescription,DeviceInfo *aDeviceInfo);
	SOCKET OpenServerConnectSocket(DeviceInfo aDevInfo);
	ReturnStatus GetMetaData(DeviceInfo aDevInfo, int32 aObjID, MetaDataObj &aMetadataObj);
	ReturnStatus ExploreServer(DeviceInfo aDevInfo, MetaDataObj &aMetadataObj);
	ReturnStatus GetDirectChilds(DeviceInfo aDevInfo, MetaDataObj &aMetadataObj,int32 startIndex, int32 limit);
	int32 GetChildCount(DeviceInfo aDevInfo,int8 *aObjId);
	MetaDataObj RetrieveObjectsFromCache(int32 aObjectID,MetaDataObj &aMetadataObj);
	void UpdateObjectsToCache(MetaDataObj &aMetadataObj,MetaDataObj aUpdateObj);
	DeviceInfo RetrieveDeviceInfo(int32 aServerID);
	void CopyObjectsToUser(MetaDataObj &aMetadataObj, cLinkedList<MetaDataObj> &aObjList,int32 startIndex, int32 limit,ContentType type);
	void DelelteServerCache(MetaDataObj &aParentObj);
	int32 ComposeAndSendGETRequest(cSocket &socket,SOCKET &s,int8 *clientAddr, int32 clientPortNo, int8 *getString);
	void UpdateCotentTypeToParentFolders(MetaDataObj &aMetadataObj, ContentType type);

	/* Class Variables. */
	static cUPnPDiscovery *cvUPnPDiscoveryPtr;
	cLinkedList<DeviceInfo> cvMediaServerList;
	cLinkedList<DeviceInfo> cvMediaRendererList;

	UPnPControlPointObserver *cvControlPointObserver;

	int32 cvServerIndex;
	int32 cvRendererIndex;

	MetaDataObj cvServerObjects;
	int32 cvActiveServerID;
	int8 cvActiveServerName[64];
	DeviceInfo cvActiveDevInfo;
	int32 cvObjectID;
	flag cvCacheServerData;
	SOCKET cvActiveServerSockID;

	cLinkedList<MediaObj> cvSongsList;
	cLinkedList<MediaObj> cvVideosList;
	cLinkedList<MediaObj> cvPicturesList;

	os_mutex cvMutex;

	int32 cvXmlParserCount;
};

#endif /* UPNP_CONTROL_POINT_H_INCLUDED */

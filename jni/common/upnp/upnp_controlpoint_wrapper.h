
#ifndef UPNP_CONTROLPOINT_WRAPPER_H_INCLUDED
#define UPNP_CONTROLPOINT_WRAPPER_H_INCLUDED

#include "../include/basic_datatypes.h"
#include "../../os/os.h"
#include "../include/return_codes.h"
#include "../log/log.h"
#include "../util/linkedlist.h"
#include "upnp_control_point.h"


class UPNPControlPointWrapper : public UPnPControlPointObserver
{
  public:
	UPNPControlPointWrapper();
	~UPNPControlPointWrapper();

	void startUPNPControlPoint();
	static void* runControlPoint(void*);
	void StopControlPoint();

	cLinkedList<MediaObj>* GetAllSongs(int serverId);
	cLinkedList<MediaObj>* GetAllVideos(int serverId);
	cLinkedList<MediaObj>* GetAllPictures(int serverId);
	ReturnStatus GetMusicObjects(int objectId, cLinkedList<MetaDataObj> &metadataList, int startIndex, int limit);
	ReturnStatus GetVideoObjects(int objectId, cLinkedList<MetaDataObj> &metadataList, int startIndex, int limit);
	ReturnStatus GetPictureObjects(int objectId, cLinkedList<MetaDataObj> &metadataList, int startIndex, int limit);
	int32 GetDuration(int objectId);
	int32 GetNumOfChilds(int objectId);
	/* Pure Virtual Function from UPnPControlPointObserver */
	void CPEventReceived(CPObject event);

	ReturnStatus SelectServer(int serverId);
	void ClearServerSelection();
	ReturnStatus SelectRenderer(int rendererId);
	void GetObjects(int serverId, int parentObjId);

	static cUPnPControlPoint *cvUPnPCPPtr;
};

#endif /* UPNP_CONTROLPOINT_WRAPPER_H_INCLUDED */

#include "upnp_controlpoint_wrapper.h"

cUPnPControlPoint *UPNPControlPointWrapper::cvUPnPCPPtr = NULL;

void notifyCPServerAdded(char *serverName, int serverId);
void notifyCPServerRemoved(int serverId);

UPNPControlPointWrapper::UPNPControlPointWrapper()
{
}

UPNPControlPointWrapper::~UPNPControlPointWrapper()
{
}

void UPNPControlPointWrapper::startUPNPControlPoint()
{
	cvUPnPCPPtr = os_new(cUPnPControlPoint,());
	if(NULL == cvUPnPCPPtr){
		  LOGE("UPNPControlPointWrapper:startUPNPControlPoint cUPnPControlPoint creation Failed.Hence exiting from here itself.. ");
		  return;
	}
	SSDPInfoStruct sSSDPDisInfoStruct = {"239.255.255.250",1900,60,"Android",2,3,"Y-Player-UPnPStack",1,0,48600};
	cvUPnPCPPtr->SetSSDPInfo(sSSDPDisInfoStruct);
	cvUPnPCPPtr->RegisterControlPointObserver((UPnPControlPointObserver*)this);
	/* Now Create the New Thread which will listen for the user input and perfroms user selected operations.. */
	if(-1 == Create_New_Thread(UPNPControlPointWrapper::runControlPoint,(void*)cvUPnPCPPtr,THREAD_PRIORITY_IS_NORMAL)){
		LOGE("UPNPControlPointWrapper:startUPNPControlPoint Failed to Create the runControlPoint Thread.. Hence Exiting from here itself..");
		return ;
	}
	return;
}

void* UPNPControlPointWrapper::runControlPoint(void *aInPtr)
{
	cUPnPControlPoint *upnpCPPtr = NULL;

	if(NULL == aInPtr){
		LOGE("userInteration: Unable to get the pointer to cUPnPControlPoint. So no point in continueing. Hence Exiting.. ");
		return NULL;
	}
	LOGD("UPNPControlPointWrapper:runControlPoint: Starting the ControlPoint");
	upnpCPPtr = (cUPnPControlPoint*)aInPtr;
	cvUPnPCPPtr->StartControlPoint();
	LOGD("UPNPControlPointWrapper:runControlPoint: Exiting the ControlPoint");
}

ReturnStatus UPNPControlPointWrapper::SelectServer(int serverId)
{
	if(cvUPnPCPPtr != NULL && SUCCESS != cvUPnPCPPtr->SelectServer(serverId,false)){
		LOGE("UPNPControlPointWrapper::SelectServer Some issue with Selection of the Server..\n");
		return FAILURE;
	}
	return SUCCESS;
}

void UPNPControlPointWrapper::ClearServerSelection()
{
	if(cvUPnPCPPtr != NULL){
		LOGE("UPNPControlPointWrapper::ClearServerSelection Calling cvUPnPCPPtr->ClearServerSelection\n");
		cvUPnPCPPtr->ClearServerSelection();
	}
}

ReturnStatus UPNPControlPointWrapper::SelectRenderer(int rendererId)
{
	return FAILURE;
}

cLinkedList<MediaObj>*
UPNPControlPointWrapper::GetAllSongs(int serverId)
{
	return cvUPnPCPPtr->GetAllSongs(serverId);
}

cLinkedList<MediaObj>*
UPNPControlPointWrapper::GetAllVideos(int serverId)
{
	return cvUPnPCPPtr->GetAllVideos(serverId);
}

cLinkedList<MediaObj>*
UPNPControlPointWrapper::GetAllPictures(int serverId)
{
	return cvUPnPCPPtr->GetAllPictures(serverId);
}

ReturnStatus
UPNPControlPointWrapper::GetMusicObjects(int objectId, cLinkedList<MetaDataObj> &metadataList, int startIndex, int limit)
{
	return cvUPnPCPPtr->GetObjects(objectId,metadataList,startIndex,limit,MUSIC);
}

ReturnStatus
UPNPControlPointWrapper::GetVideoObjects(int objectId, cLinkedList<MetaDataObj> &metadataList, int startIndex, int limit)
{
	return cvUPnPCPPtr->GetObjects(objectId,metadataList,startIndex,limit,VIDEO);
}

ReturnStatus
UPNPControlPointWrapper::GetPictureObjects(int objectId, cLinkedList<MetaDataObj> &metadataList, int startIndex, int limit)
{
	return cvUPnPCPPtr->GetObjects(objectId,metadataList,startIndex,limit,PICTURES);
}

int32
UPNPControlPointWrapper::GetDuration(int objectId)
{
	return cvUPnPCPPtr->GetDuration(objectId);
}

int32
UPNPControlPointWrapper::GetNumOfChilds(int objectId)
{
	//LOGD("UPNPControlPointWrapper::GetNumOfChilds IN");
	return cvUPnPCPPtr->GetNumOfChilds(objectId);
	//LOGD("UPNPControlPointWrapper::GetNumOfChilds OUT");
}

void UPNPControlPointWrapper::CPEventReceived(CPObject event)
{
	if(SERVER_ADDED == event.EventID){
		/* Call back to Java saying about the server Name which is added. (event.DeviceName) */
		notifyCPServerAdded(event.DeviceName,event.DeviceID);
	}else if(RENDERER_ADDED == event.EventID){
		/* Call back to Java saying about the Renderer Name which is added. (event.DeviceName) */
	}else if(SERVER_REMOVED == event.EventID){
		/* Intimating to Java about Removing the server from the list. */
	}else if(RENDERER_REMOVED == event.EventID){
		/* Intimating to Java about Removing the Renderer from the list. */
		notifyCPServerRemoved(event.DeviceID);
	}else{
		/* NOthing to do here.. Just ignore.. */
	}
}

void UPNPControlPointWrapper::StopControlPoint()
{
	if(NULL != cvUPnPCPPtr){
		cvUPnPCPPtr->StopControlPoint();
		os_delete (cvUPnPCPPtr);
		cvUPnPCPPtr = NULL;
	}
}

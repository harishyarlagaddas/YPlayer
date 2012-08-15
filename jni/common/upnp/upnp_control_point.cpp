
#include "upnp_control_point.h"
#include "upnp_control_point_actions.incl"

/* Create the pointers to Discovery and Description classes. */
cUPnPDiscovery* cUPnPControlPoint::cvUPnPDiscoveryPtr = NULL;

cUPnPControlPoint::cUPnPControlPoint()
{
	cvUPnPDiscoveryPtr = os_new(cUPnPDiscovery,());

	os_mutex_attribute mutexAttr;
	pthread_mutexattr_init(&mutexAttr);
	os_mutex_initialize(&cvMutex,&mutexAttr);

	cvControlPointObserver = NULL;
	cvServerIndex = 0;
	cvRendererIndex = 0;
	cvObjectID = 0;
	cvActiveServerID = -1;
	cvActiveServerSockID = -1;
	cvCacheServerData = false;
	os_memset(&cvActiveServerName,0,sizeof(cvActiveServerName));
	os_memset(&cvServerObjects,0,sizeof(cvServerObjects));
	os_memset(&cvActiveDevInfo,0,sizeof(cvActiveDevInfo));

	cvXmlParserCount = 0;
}

cUPnPControlPoint::~cUPnPControlPoint()
{
	if(cvUPnPDiscoveryPtr){
		os_delete(cvUPnPDiscoveryPtr);
		cvUPnPDiscoveryPtr = NULL;
	}
	os_mutex_destroy(&cvMutex);
}

void
cUPnPControlPoint::DelelteServerCache(MetaDataObj &aParentObj)
{
	int32 count;
	MetaDataObj tempObj;
	if(NULL != aParentObj.ChildList){
		for(count = 1; count <= aParentObj.ChildList->GetNumberOfElements(); count++){
			os_memset(&tempObj,0,sizeof(tempObj));
			aParentObj.ChildList->GetElementAtIndex(tempObj,count);
			if(true == tempObj.Container){
				DelelteServerCache(tempObj);
			}
		}
		os_delete(aParentObj.ChildList);
		aParentObj.ChildList = NULL;
		os_memset((void*)&aParentObj,0,sizeof(aParentObj));
	}
}

void
cUPnPControlPoint::RegisterControlPointObserver(UPnPControlPointObserver *aObserver)
{
	cvControlPointObserver = aObserver;
}

void
cUPnPControlPoint::SetSSDPInfo(const SSDPInfoStruct &aSSDPInfo)
{
	cvUPnPDiscoveryPtr->SetSSDPInfo(aSSDPInfo);
}

ReturnStatus
cUPnPControlPoint::StartControlPoint()
{

	/* Before Starting any of the operations, it's better to register the signal handlers for handling the odd cases. */
	os_signal_register(OS_SIGINT,cUPnPControlPoint::HandleSignal);
	os_signal_register(OS_SIGABRT,cUPnPControlPoint::HandleSignal);
	os_signal_register(OS_SIGTERM,cUPnPControlPoint::HandleSignal);
	os_signal_register(OS_SIGPIPE,cUPnPControlPoint::HandleSignal);

	/* After this before starting the description, refresh the active ip addresses. Because description and discovery will use these
	   ip list for their operations. */
	//LOGD("cUPnPControlPoint::StartUPnPOperations Refreshing for Active IP Addresses");
	cvUPnPDiscoveryPtr->RefreshAtiveIPAddress();
	/* Now Configure the DiscoveryModule to broadcast the message for ControlPoint.. (Not to the Server.. :)) */
	cvUPnPDiscoveryPtr->SetDiscoveryForControlPoint();
	cvUPnPDiscoveryPtr->RegisterControlPointInterfacePtr((UPnPControlPointInterface*)this);
	/* After this you can start discovery. As description is already running to handle the requsts comming from clients based on
	  broadcast messages broadcasted from discovery. */
	//LOGD("cUPnPControlPoint::StartUPnPOperations Starting Device Discovery");
	cvUPnPDiscoveryPtr->StartUPnPDiscovery(); /* This is going to be the blocking call until the sever is active.. */
	return SUCCESS;
}

ReturnStatus
cUPnPControlPoint::ParseURL(int8 *aUrl, int8* aAddr, int32 &aPortNo, int8* aString)
{
	int8 portNoStr[10] = {'\0'};
	int8 *temp = NULL, *temp1 = NULL;

	temp = os_strcasestr(aUrl,"http://");
	if(NULL == temp){
		return FAILURE;
	}
	temp += 7;

	temp1 = os_strcasestr(temp,":");
	if(NULL == temp1){
		return FAILURE;
	}

	/*copy the clients ip address.. */
	os_memcpy(aAddr,temp,temp1-temp);

	temp1++;
	temp = os_strcasestr(temp1,"/");
	if(NULL == temp){
		return FAILURE;
	}

	/* Copy the Client's Port No.. */
	os_memcpy(portNoStr,temp1,temp-temp1);
	aPortNo = os_atoi(portNoStr);
	//temp++;

	os_strcpy(aString,temp);

	return SUCCESS;
}

ReturnStatus
cUPnPControlPoint::ParseDeviceInfo(int8 *aDeviceDescription,DeviceInfo *aDeviceInfo)
{
	int8 *tempPtr = NULL, tagValue[128] = {'\0'};
	int32 count = 0, noServices = 0;
	ReturnStatus status = SUCCESS;
	TiXmlDocument xmlParser;
	TiXmlNode* serviceList = NULL;

	xmlParser.Parse(aDeviceDescription);

	os_memset(tagValue,0,sizeof(tagValue));
	if(SUCCESS != XmlUtils::GetTagValue(xmlParser,(int8*)"deviceType",tagValue)){
		LOGE("cUPnPControlPoint::ParseDeviceInfo Unable to get \"deviceType\" from device description");
		status = FAILURE;
		goto ParseDeviceInfo_Error_Exit;
	}

	if(0 != os_strncasecmp(tagValue,DIGITAL_MEDIA_SERVER_STRING,os_strlen((int8*)DIGITAL_MEDIA_SERVER_STRING)) &&
	   0 != os_strncasecmp(tagValue,DIGITAL_MEDIA_RENDERER_STRING,os_strlen((int8*)DIGITAL_MEDIA_RENDERER_STRING))){
		LOGE("cUPnPControlPoint::ParseDeviceInfo \"deviceType\" didn't match with server or renderer");
		status = FAILURE;
		goto ParseDeviceInfo_Error_Exit;
	}
	/* Fill the Device Friendly Name */
	os_memset(tagValue,0,sizeof(tagValue));
	if(SUCCESS != XmlUtils::GetTagValue(xmlParser,(int8*)"friendlyName",tagValue)){
		LOGE("cUPnPControlPoint::ParseDeviceInfo Unable to get \"friendlyName\" from device description");
		status = FAILURE;
		goto ParseDeviceInfo_Error_Exit;
	}
	os_strncpy(aDeviceInfo->DeviceName,tagValue,sizeof(aDeviceInfo->DeviceName)-1);

	/* Fill the UUID */
	os_memset(tagValue,0,sizeof(tagValue));
	if(SUCCESS != XmlUtils::GetTagValue(xmlParser,(int8*)"UDN",tagValue)){
		LOGE("cUPnPControlPoint::ParseDeviceInfo Unable to get \"UDN\" from device description");
		status = FAILURE;
		goto ParseDeviceInfo_Error_Exit;
	}

	tempPtr = os_strcasestr(tagValue,"uuid");
	if(NULL == tempPtr){
		LOGE("cUPnPControlPoint::ParseDeviceInfo Unable to get \"uuid\" from device description");
		status = FAILURE;
		goto ParseDeviceInfo_Error_Exit;
	}
	tempPtr += 4;
	while(' ' == *tempPtr || ':' == *tempPtr){
		tempPtr++;
	}
	os_strncpy(aDeviceInfo->DeviceUUID,tempPtr,sizeof(aDeviceInfo->DeviceUUID)-1);

	/* Filling the URL base. */
	os_memset(tagValue,0,sizeof(tagValue));
	if(SUCCESS == XmlUtils::GetTagValue(xmlParser,(int8*)"URLBase",tagValue)){
		os_strncpy(aDeviceInfo->BaseURL,tagValue,sizeof(aDeviceInfo->BaseURL)-1);
	}

	os_memcpy(&aDeviceInfo->LastActiveTime,os_get_time(),sizeof(aDeviceInfo->LastActiveTime));

	/* Now start Enumerating the services.. */
	serviceList = XmlUtils::GetTag(xmlParser,(int8*)"serviceList");
	if(serviceList != NULL){
		noServices = XmlUtils::GetNumOfChilds(serviceList);
		for(count = 1; count <= noServices; count++){
			ServiceInfo serviceInfo;
			os_memset(&serviceInfo,0,sizeof(serviceInfo));
			TiXmlNode* sInfo = NULL;
			sInfo = XmlUtils::GetSubTagAtIndex(serviceList,count);
			if(NULL == sInfo){
				LOGE("cUPnPControlPoint::ParseDeviceInfo Unable to get subtag \"serviceList\" at the index [%d] from device description",count);
				status = FAILURE;
				goto ParseDeviceInfo_Error_Exit;
			}

			/* Fill the Service ID */
			os_memset(tagValue,0,sizeof(tagValue));
			if(SUCCESS != XmlUtils::GetTagValue(sInfo,(int8*)"serviceType",tagValue)){
				LOGE("cUPnPControlPoint::ParseDeviceInfo Unable to get \"serviceType\" from device description");
				status = FAILURE;
				goto ParseDeviceInfo_Error_Exit;
			}

			if(os_strcasestr(tagValue,CONTENT_DIRECTORY_SERVICE_TYPE)){
				serviceInfo.ServiceID = CONTENT_DIRECTORY_SERVICE_ID;
			}else if(os_strcasestr(tagValue,CONNECTION_MANAGER_SERVICE_TYPE)){
				serviceInfo.ServiceID = CONNECTION_MANAGER_SERVICE_ID;
			}else{
				/* As of Now we don't need any other service... :) */
				continue;
			}

			/* Fill the SCPDURL */
			os_memset(tagValue,0,sizeof(tagValue));
			if(SUCCESS != XmlUtils::GetTagValue(sInfo,(int8*)"SCPDURL",tagValue)){
				LOGE("cUPnPControlPoint::ParseDeviceInfo Unable to get \"SCPDURL\" from device description");
				status = FAILURE;
				goto ParseDeviceInfo_Error_Exit;
			}
			os_strncpy(serviceInfo.SCPDURL,tagValue,sizeof(serviceInfo.SCPDURL)-1);

			/* Fill the controlURL */
			os_memset(tagValue,0,sizeof(tagValue));
			if(SUCCESS != XmlUtils::GetTagValue(sInfo,(int8*)"controlURL",tagValue)){
				LOGE("cUPnPControlPoint::ParseDeviceInfo Unable to get \"controlURL\" from device description");
				status = FAILURE;
				goto ParseDeviceInfo_Error_Exit;
			}
			os_strncpy(serviceInfo.ControlURL,tagValue,sizeof(serviceInfo.ControlURL)-1);

			/* Fill the eventSubURL */
			os_memset(tagValue,0,sizeof(tagValue));
			if(SUCCESS != XmlUtils::GetTagValue(sInfo,(int8*)"eventSubURL",tagValue)){
				LOGE("cUPnPControlPoint::ParseDeviceInfo Unable to get \"eventSubURL\" from device description");
				status = FAILURE;
				goto ParseDeviceInfo_Error_Exit;
			}
			os_strncpy(serviceInfo.EventingURL,tagValue,sizeof(serviceInfo.EventingURL)-1);

			/* Now Check whether the ServiceList pointer is NULL or not. If it is NULL then allocate Memory for that.. */
			if(NULL == aDeviceInfo->ServiceList){
				aDeviceInfo->ServiceList = (cLinkedList<ServiceInfo>*)os_new(cLinkedList<ServiceInfo>,());
				if(NULL == aDeviceInfo->ServiceList){
					LOGE("cUPnPControlPoint::ParseDeviceInfo Unable to Allocate the Memory for ServiceList");
					status = FAILURE;
					goto ParseDeviceInfo_Error_Exit;
				}
			}

			/* Now filling of all the info in to the ServiceInfo structure is done. Now push this in to the DeviceInfo structure... */
			if(SUCCESS != aDeviceInfo->ServiceList->Add(serviceInfo)){
				LOGE("cUPnPControlPoint::ParseDeviceInfo Error while adding Service Info to serviceList");
				status = FAILURE;
				goto ParseDeviceInfo_Error_Exit;
			}
		}
	}
	status = SUCCESS;
ParseDeviceInfo_Error_Exit:
	xmlParser.Clear();
	xmlParser.~TiXmlDocument();
	return status;
}

int32
cUPnPControlPoint::ComposeAndSendGETRequest(cSocket &socket,SOCKET &s,int8 *clientAddr, int32 clientPortNo, int8 *getString)
{
	int8 ownAddr[17] = {'\0'}, getRequest[HTTP_REQUEST_SIZE] = {'\0'},getRequestRecvHeader[HTTP_HEADER_SIZE] = {'\0'};
	int32 ownPortNo, contentLength = -1;

	/* Initialize the Socket Params for creating the corresponding socket.. */
	socket.InitializeParams(clientAddr,clientPortNo,this);
	s= socket.CreateTCPClientSocket();

	/* Get the own address of the socket.. */
	if(SUCCESS != os_get_SocketAddress(s,ownAddr,&ownPortNo)){
		LOGE("cUPnPControlPoint::ComposeAndSendGETRequest: get Sock Addr Error");
		socket.CloseSocket(s);
		return -1;
	}

	/* Compose the GET request.. */
	if(SUCCESS != cHttpUtils::ComposeGETRequest(getString,ownAddr,ownPortNo,(int8*)"en-us",getRequest,sizeof(getRequest))){
		LOGE("cUPnPControlPoint::ComposeAndSendGETRequest: Compose GET Request Error");
		socket.CloseSocket(s);
		return -1;
	}

	/* Now send the GET request to the client. */
	if(os_strlen(getRequest) != socket.Send(s,getRequest,os_strlen(getRequest))){
		LOGE("cUPnPControlPoint::ComposeAndSendGETRequest: Socket Send Error");
		socket.CloseSocket(s);
		return -1;
	}

	/* Now receive the response header for the GET request sent above.. */
	if(FAILURE == cHttpUtils::ReceiveHeader(s,getRequestRecvHeader,sizeof(getRequestRecvHeader))){
		LOGE("cUPnPControlPoint::ComposeAndSendGETRequest: Recv Header Error");
		socket.CloseSocket(s);
		return -1;
	}

	/* Now Parse the response header and if it is fine then go ahead to receive the complete deviceinfo.xml file.. */
	if(NULL == os_strcasestr(getRequestRecvHeader,"HTTP/1.1 200 OK")){
		LOGE("cUPnPControlPoint::ComposeAndSendGETRequest: HTTP Error code Error");
		socket.CloseSocket(s);
		return -1;
	}

	/* As of now we don't support chunked transfer. When we support we can enable it.. */
	if((NULL == os_strcasestr(getRequestRecvHeader,"CONTENT-LENGTH")) && (NULL == os_strstr(getRequestRecvHeader,"chunked"))){
		LOGE("cUPnPControlPoint::ComposeAndSendGETRequest: No Content Length Error");
		socket.CloseSocket(s);
		return -1;
	}

	/* Now extract the content length */
	contentLength = cHttpUtils::ExtractContentLength(getRequestRecvHeader);
	if(FAILURE == contentLength){
		LOGE("cUPnPControlPoint::ComposeAndSendGETRequest: Extracting Content Length Error");
		socket.CloseSocket(s);
		return -1;
	}
	return contentLength;
}

ReturnStatus
cUPnPControlPoint::GetDeviceInfo(int8 *aUrl, int8* aUUID, int32 aLifeTime,DeviceInfo *aDeviceInfo)
{

	int8 clientAddr[17] = {'\0'}, *deviceDescriptionPtr = NULL, getString[64] = {'\0'};
	int32 clientPortNo, contentLength = 0, recvBytes = 0;
	cSocket socket;
	SOCKET s;
	ReturnStatus status = SUCCESS;

	/* First parse the URL to get the ip addr and port no. */
	if(SUCCESS != ParseURL(aUrl,clientAddr,clientPortNo,getString)){
		LOGE("cUPnPControlPoint::GetDeviceInfo: ParseURL Error");
		status = FAILURE;
		goto GetDeviceInfo_Error_Exit;
	}

	contentLength = ComposeAndSendGETRequest(socket,s,clientAddr,clientPortNo,getString);
	if(-1 == contentLength){
		LOGE("cUPnPControlPoint::GetDeviceInfo: Error in composing and sending the GET request");
		status = FAILURE;
		goto GetDeviceInfo_Error_Exit;
	}
		
	deviceDescriptionPtr = (int8*)os_malloc(contentLength+1);
	if(NULL == deviceDescriptionPtr){
		LOGE("cUPnPControlPoint::GetDeviceInfo: Memory Allocation Error");
		status = FAILURE;
		goto GetDeviceInfo_Error_Exit;
	}

	recvBytes = socket.Receive(s,deviceDescriptionPtr,contentLength);
	if(contentLength != recvBytes){
		LOGE("cUPnPControlPoint::GetDeviceInfo: Recv Error. Received Bytes [%d] ContentLength [%d]",recvBytes,contentLength);
		status = FAILURE;
		goto GetDeviceInfo_Error_Exit;
	}

	if(SUCCESS != ParseDeviceInfo(deviceDescriptionPtr,aDeviceInfo)){
		LOGE("cUPnPControlPoint::GetDeviceInfo: ParseDeviceInfo Error");
		status = FAILURE;
		goto GetDeviceInfo_Error_Exit;
	}

	/* Here add the device location address and the Max time val.. */
	os_strncpy(aDeviceInfo->DeviceIP,clientAddr,sizeof(aDeviceInfo->DeviceIP)-1);
	aDeviceInfo->DevicePortNo = clientPortNo;
	aDeviceInfo->ValidDuration = aLifeTime;

	status = SUCCESS;

GetDeviceInfo_Error_Exit:
	//LOG_CONSOLE("cUPnPControlPoint::GetAndAddDevice: ERROR EXIT");
	socket.CloseSocket(s);
	if(deviceDescriptionPtr){
		os_delete(deviceDescriptionPtr);
	}
	return status;
}

ReturnStatus
cUPnPControlPoint::AddServer(int8 *aUrl, int8* aUUID, int32 aLifeTime)
{
	LOGD("cUPnPControlPoint::AddServer IN URL = %s",aUrl);
	int8 ip[32] = {'\0'}, getString[128] = {'\0'};
	int32 count = 0, noDevices = 0, port = 0;
	DeviceInfo devInfo;
	CPObject obj;
	ReturnStatus status = SUCCESS;

	////LOGD("cUPnPControlPoint::AddServer Locking the Mutex-cvMutex");
	/* Lock the Mutex before Adding.. */
	os_mutex_lock(&cvMutex);
	////LOGD("cUPnPControlPoint::AddServer Locking the Mutex Done-cvMutex");
	/* Now start Enumerating the services.. */
	noDevices = cvMediaServerList.GetNumberOfElements();
	for(count = 1; count <= noDevices; count++){
		os_memset(&devInfo,0,sizeof(devInfo));
		if(SUCCESS == cvMediaServerList.GetElementAtIndex(devInfo,count)){
			if( 0 == os_strncasecmp(aUUID,devInfo.DeviceUUID,os_strlen(aUUID))){
				/* Device is already added in the list.. So just update lifetime value and the last actuve time in the structure..No Need to get deviceinfo.xml again */
				devInfo.ValidDuration = aLifeTime;
				os_memcpy(&devInfo.LastActiveTime,os_get_time(),sizeof(devInfo.LastActiveTime));

				//LOGD("cUPnPControlPoint::AddServer Device is already added. Hence Ignore for now.. ");
				if(SUCCESS != cvMediaServerList.RemoveElementAtIndex(count)){
					LOGE("cUPnPControlPoint::AddServer Failed to remove the server from the list.. Just before updating");
				}
				if(SUCCESS != cvMediaServerList.Add(devInfo)){
					LOGE("cUPnPControlPoint::AddServer Failed to add the server from the list.. Just before updating");
				}
				/* Return from here it self.. */
				status = SUCCESS;
				goto AddServer_Exit;
			}
		}
	}
	os_memset(&devInfo,0,sizeof(devInfo));
	if(SUCCESS != GetDeviceInfo(aUrl,aUUID,aLifeTime,&devInfo)){
		LOGE("cUPnPControlPoint::AddServer Error While Getting the Device Info. Hence not able to add the server!!");
		status = FAILURE;
		goto AddServer_Exit;
	}

	/* Fill the data to intimate the Observer.. */
	os_memset(&obj,0,sizeof(obj));
	os_strncpy(obj.DeviceName,devInfo.DeviceName,sizeof(obj.DeviceName));
	obj.DeviceID = obj.ObjID = devInfo.deviceID = cvServerIndex++;
	obj.EventID = SERVER_ADDED;

	/* Copy the device URL for further use.. */
	ParseURL(aUrl,ip,port,getString);
	if(os_strlen(getString)){
		os_strncpy(devInfo.DeviceStringInURL,getString,sizeof(devInfo.DeviceStringInURL));
	}

	cvMediaServerList.Add(devInfo);

	/* Now Intimate the Observer.. */
	if(cvControlPointObserver){
		//LOGD("cUPnPControlPoint::AddServer Calling the controlpoint observer to add the device %s",obj.DeviceName);
		cvControlPointObserver->CPEventReceived(obj);
	}
	status = SUCCESS;
AddServer_Exit:
	////LOGD("cUPnPControlPoint::AddServer Unocking the Mutex-cvMutex");
	os_mutex_unlock(&cvMutex);
	////LOGD("cUPnPControlPoint::AddServer Unlocking the Mutex Done-cvMutex");
	//LOGD("cUPnPControlPoint::AddServer OUT. status = %d",status);
	return status;
}

ReturnStatus
cUPnPControlPoint::AddRenderer(int8 *aUrl, int8* aUUID, int32 aLifeTime)
{
	int32 count = 0, noDevices = 0;
	DeviceInfo devInfo;
	CPObject obj;
	ReturnStatus status = SUCCESS;
	////LOGD("cUPnPControlPoint::AddRenderer Locking the Mutex-cvMutex");
	/* Lock the Mutex before Adding.. */
	os_mutex_lock(&cvMutex);
	////LOGD("cUPnPControlPoint::AddRenderer Locking the Mutex Done-cvMutex");

	/* Now start Enumerating the services.. */
	noDevices = cvMediaRendererList.GetNumberOfElements();
	for(count = 1; count <= noDevices; count++){
		os_memset(&devInfo,0,sizeof(devInfo));
		if(SUCCESS == cvMediaRendererList.GetElementAtIndex(devInfo,count)){
			//LOGD("cUPnPControlPoint::AddRenderer Incomming UUID [%s] Fetched UUID [%s]",aUUID,devInfo.DeviceUUID);
			if( 0 == os_strncasecmp(aUUID,devInfo.DeviceUUID,os_strlen(aUUID))){
				/* Device is already added in the list.. So just update lifetime value and the last actuve time in the structure..No Need to get deviceinfo.xml again */
				devInfo.ValidDuration = aLifeTime;
				os_memcpy(&devInfo.LastActiveTime,os_get_time(),sizeof(devInfo.LastActiveTime));
				if(SUCCESS != cvMediaRendererList.RemoveElementAtIndex(count)){
					LOGE("cUPnPControlPoint::AddRenderer Failed to remove the server from the list.. Just before updating");
				}
				if(SUCCESS != cvMediaRendererList.Add(devInfo)){
					LOGE("cUPnPControlPoint::AddRenderer Failed to add the server from the list.. Just before updating");
				}
				/* Return from here it self.. */
				status = SUCCESS;
				goto AddRenderer_Exit;
			}
		}
	}

	os_memset(&devInfo,0,sizeof(devInfo));
	if(SUCCESS != GetDeviceInfo(aUrl,aUUID,aLifeTime,&devInfo)){
		status = FAILURE;
		goto AddRenderer_Exit;
	}

	/* Fill the data to intimate the Observer.. */
	os_memset(&obj,0,sizeof(obj));
	os_strncpy(obj.DeviceName,devInfo.DeviceName,sizeof(obj.DeviceName));
	obj.DeviceID = obj.ObjID = devInfo.deviceID = cvRendererIndex++;
	obj.EventID = RENDERER_ADDED;

	cvMediaRendererList.Add(devInfo);
	//os_mutex_unlock(&cvMutex);

	/* Now Intimate the Observer.. */
	if(cvControlPointObserver){
		cvControlPointObserver->CPEventReceived(obj);
	}

	status = SUCCESS;
AddRenderer_Exit:
	////LOGD("cUPnPControlPoint::AddRenderer Unlocking the Mutex-cvMutex");
	os_mutex_unlock(&cvMutex);
	////LOGD("cUPnPControlPoint::AddRenderer Unlocking the Mutex Done-cvMutex");
	return status;
}

ReturnStatus
cUPnPControlPoint::RemoveServer(int8* aUUID)
{
	int32 count = 0, noDevices = 0;
	DeviceInfo devInfo;
	CPObject obj;
	ReturnStatus status = SUCCESS;
	////LOGD("cUPnPControlPoint::RemoveServer Locking the Mutex-cvMutex");
	/* Lock the Mutex before Adding.. */
	os_mutex_lock(&cvMutex);
	////LOGD("cUPnPControlPoint::RemoveServer Locking the Mutex Done-cvMutex");
	/* Now start Enumerating the services.. */
	noDevices = cvMediaServerList.GetNumberOfElements();
	for(count = 1; count <= noDevices; count++){
		os_memset(&devInfo,0,sizeof(devInfo));
		if(SUCCESS == cvMediaServerList.GetElementAtIndex(devInfo,count)){
			if( 0 == os_strncasecmp(aUUID,devInfo.DeviceUUID,os_strlen(aUUID))){
				/* Device Found. Remove from the list...*/
				LOGD("cUPnPControlPoint::RemoveServer Device Found.. So removing from the list.. ");
				if(SUCCESS != cvMediaServerList.RemoveElementAtIndex(count)){
					LOGE("cUPnPControlPoint::RemoveServer Failed to remove the server from the list..");
				}
				os_memset(&obj,0,sizeof(obj));
				obj.DeviceID = obj.ObjID = devInfo.deviceID;
				obj.EventID = SERVER_REMOVED;

				/* Now Intimate the Observer.. */
				if(cvControlPointObserver){
					cvControlPointObserver->CPEventReceived(obj);
				}

				/* Return from here it self.. */
				status = SUCCESS;
				goto RemoveServer_Exit;
			}
		}
	}
	status = SUCCESS;
RemoveServer_Exit:
	////LOGD("cUPnPControlPoint::RemoveServer Unlocking the Mutex-cvMutex");
	os_mutex_unlock(&cvMutex);
	////LOGD("cUPnPControlPoint::RemoveServer Unlocking the Mutex Done-cvMutex");
	return status;
}

ReturnStatus
cUPnPControlPoint::RemoveRenderer(int8* aUUID)
{
	int32 count = 0, noDevices = 0;
	DeviceInfo devInfo;
	CPObject obj;
	ReturnStatus status = SUCCESS;

	/* Lock the Mutex before Adding.. */
	////LOGD("cUPnPControlPoint::RemoveRenderer Locking the Mutex-cvMutex");
	os_mutex_lock(&cvMutex);
	////LOGD("cUPnPControlPoint::RemoveRenderer Locking the Mutex Done-cvMutex");
	/* Now start Enumerating the services.. */
	noDevices = cvMediaRendererList.GetNumberOfElements();
	for(count = 1; count <= noDevices; count++){
		os_memset(&devInfo,0,sizeof(devInfo));
		if(SUCCESS == cvMediaRendererList.GetElementAtIndex(devInfo,count)){
			if( 0 == os_strncasecmp(aUUID,devInfo.DeviceUUID,os_strlen(aUUID))){
				/* Device Found. Remove from the list...*/
				LOGD("cUPnPControlPoint::RemoveRenderer Device Found.. So removing from the list.. ");
				if(SUCCESS != cvMediaRendererList.RemoveElementAtIndex(count)){
					LOGE("cUPnPControlPoint::RemoveRenderer Failed to remove the Renderer from the list..");
				}

				os_memset(&obj,0,sizeof(obj));
				obj.DeviceID = obj.ObjID = devInfo.deviceID;
				obj.EventID = RENDERER_REMOVED;

				/* Now Intimate the Observer.. */
				if(cvControlPointObserver){
					cvControlPointObserver->CPEventReceived(obj);
				}

				/* Return from here it self.. */
				/* Return from here it self.. */
				status = SUCCESS;
				goto RemoveRenderer_Exit;
			}
		}
	}
	status = SUCCESS;
RemoveRenderer_Exit:
	////LOGD("cUPnPControlPoint::RemoveRenderer Unlocking the Mutex-cvMutex");
	os_mutex_unlock(&cvMutex);
	////LOGD("cUPnPControlPoint::RemoveRenderer Unlocking the Mutex Done-cvMutex");
	return status;
}

ReturnStatus cUPnPControlPoint::GetMetaData(DeviceInfo aDevInfo, int32 aObjID, MetaDataObj &aMetadataObj)
{
	//LOGD("cUPnPControlPoint::GetMetaData IN ObjectID = %d",aObjID);
	int8 postReq[HTTP_REQUEST_SIZE] = {'\0'},
	     browseMetadataReq[738] = {'\0'},
	     postRequestRecvHeader[HTTP_HEADER_SIZE] = {'\0'},
	     *postReqResultPtr = NULL,
	     id[16] = {'\0'}, tagValue[128] = {'\0'}, controlURL[128] = {'\0'};

	ServiceInfo servInfo;
	TiXmlDocument xmlParser;
	TiXmlDocument xmlParser1;
	TiXmlAttribute* xmlAttr = NULL;
	TiXmlNode * xmlNode = NULL;
	int32 recvBytes = 0, contentLength = 0, servCount = 0, noAttr = 0, attrCount = 0;
	ReturnStatus status = SUCCESS;

	/* Extract the service control URL for posing the request.. */
	for(servCount = 1; servCount <= aDevInfo.ServiceList->GetNumberOfElements(); servCount++){
		os_memset(&servInfo,0,sizeof(servInfo));
		if(SUCCESS == aDevInfo.ServiceList->GetElementAtIndex(servInfo,servCount)){
			if(CONTENT_DIRECTORY_SERVICE_ID == servInfo.ServiceID){
				break;
			}
		}
	}

	if(servCount > aDevInfo.ServiceList->GetNumberOfElements()){
		/* Even the server is found.. But this server is not supporting CDS.. Hence return failure to the app.. */
		LOGE("cUPnPControlPoint:GetMetaData server doesn't support CDS. Hence returning.");
		return FAILURE;
	}
	/* Some stupid servers like PlayOn we need to add "/" infront of all requests. Otherwise it will through error.
	 * Don't know why they don't follow the guidelines. Stupid people and stupid servers...
	 */
	if(os_strlen(aDevInfo.DeviceStringInURL)){
		if(os_strcmp(aDevInfo.DeviceStringInURL,"/") == 0){
			os_strncpy(controlURL,aDevInfo.DeviceStringInURL,sizeof(controlURL));
			os_strcat(controlURL,servInfo.ControlURL);
		}else{
			os_strncpy(controlURL,servInfo.ControlURL,sizeof(controlURL));
		}
	}else{
		os_strncpy(controlURL,servInfo.ControlURL,sizeof(controlURL));
	}

	/* Fill the incomming id as string for filling it in the browseMetadataReq... */
	os_snprintf(id,sizeof(id),"%d",aObjID);
	/* Create the XML request for requesting the Metadata from the server.. */
	os_snprintf(browseMetadataReq,sizeof(browseMetadataReq),gUPnPCPActions[UPNP_CP_BROWSE_ACTION],id,gUPnPCPTags[BROWSE_METADATA],"*",0,1,"");
	/* Create the POST request for sending to the server.. */
	os_snprintf(postReq,sizeof(postReq),gUPnPCPActions[UPNP_CP_POST_REQ],controlURL,gUPnPCPTags[BROWSE_REQUEST],aDevInfo.DeviceIP,aDevInfo.DevicePortNo,os_strlen(browseMetadataReq));

	/* First send POST Request to the server. */
	if(-1 == cvActiveServerSockID){
		OpenServerConnectSocket(aDevInfo);
	}
	if(os_strlen(postReq) != cSocket::Send(cvActiveServerSockID,postReq,os_strlen(postReq))){
		/* In case Failure try opening the socket again. */
		OpenServerConnectSocket(aDevInfo);
		if(os_strlen(postReq) != cSocket::Send(cvActiveServerSockID,postReq,os_strlen(postReq))){
			LOGE("cUPnPControlPoint::GetMetaData: Error while seding the POST request over socket");
			cSocket::CloseSocket(cvActiveServerSockID);
			cvActiveServerSockID = -1;
			return FAILURE;
		}
	}

	/* Now start sending the xml request for Metadata.. */
	if(os_strlen(browseMetadataReq) != cSocket::Send(cvActiveServerSockID,browseMetadataReq,os_strlen(browseMetadataReq))){
		LOGE("cUPnPControlPoint::GetMetaData: Error while sending browseMetadataRequest over socket");
		/* Some problem in sending the post request.. */
		cSocket::CloseSocket(cvActiveServerSockID);
		cvActiveServerSockID = -1;
		return FAILURE;
	}

	/* Now receive the response header for the POST request sent above.. */
	if(FAILURE == cHttpUtils::ReceiveHeader(cvActiveServerSockID,postRequestRecvHeader,sizeof(postRequestRecvHeader))){
		LOGE("cUPnPControlPoint::GetMetaData: Recv Header Error");
		status = FAILURE;
		goto GetMetaData_Error_Exit;
	}

	/* Now Parse the response header and if it is fine then go ahead to receive the complete deviceinfo.xml file.. */
	if(NULL == os_strcasestr(postRequestRecvHeader,"HTTP/1.1 200 OK")){
		LOGE("cUPnPControlPoint::GetMetaData: HTTP Error code Error");
		status = FAILURE;
		goto GetMetaData_Error_Exit;
	}

	/* As of now we don't support chunked transfer. When we support we can enable it.. */
	if((NULL == os_strcasestr(postRequestRecvHeader,"CONTENT-LENGTH")) && (NULL == os_strstr(postRequestRecvHeader,"chunked"))){
		LOGE("cUPnPControlPoint::GetMetaData: No Content Length Error");
		status = FAILURE;
		goto GetMetaData_Error_Exit;
	}

	/* Now extract the content length */
	contentLength = cHttpUtils::ExtractContentLength(postRequestRecvHeader);
	if(FAILURE == contentLength){
		LOGE("cUPnPControlPoint::GetMetaData: Extracting Content Length Error");
		status = FAILURE;
		goto GetMetaData_Error_Exit;
	}

	postReqResultPtr = (int8*)os_malloc(contentLength+1);
	if(NULL == postReqResultPtr){
		LOGE("cUPnPControlPoint::GetMetaData: Memory Allocation Error");
		status = FAILURE;
		goto GetMetaData_Error_Exit;
	}

	recvBytes = cSocket::Receive(cvActiveServerSockID,postReqResultPtr,contentLength);
	if(contentLength != recvBytes){
		LOGE("cUPnPControlPoint::GetMetaData: Recv Error. Received Bytes [%d] ContentLength [%d]",recvBytes,contentLength);
		status = FAILURE;
		goto GetMetaData_Error_Exit;
	}

	if(CONNECTION_CLOSE == cHttpUtils::GetConnectionType(postRequestRecvHeader)){
		cSocket::CloseSocket(cvActiveServerSockID);
		cvActiveServerSockID = -1;
	}
	//LOG_CONSOLE("POST Request result: \n %s",postReqResultPtr);

	/* Now we got the response. Need to parse and extract the required Metada.. */
	xmlParser.Parse(postReqResultPtr);

	os_memset(tagValue,0,sizeof(tagValue));
	if(SUCCESS != XmlUtils::GetTagValue(xmlParser,(int8*)"NumberReturned",tagValue)){ /* If we don't have ths tag means then there is something wrong somewhere.. Find out.. */
		LOGE("cUPnPControlPoint::GetMetaData: Error while getting \"NumberReturned\" tag from xml");
		status = FAILURE;
		goto GetMetaData_Error_Exit;
	}

	if(1 != os_atoi(tagValue)){ /* In case of get Metadata the NumberReturned should be equal to 1. Otherwise there might be something wrong.. */
		LOGE("cUPnPControlPoint::GetMetaData: Error while converting \"NumberReturned\" to Integer from String");
		status = FAILURE;
		goto GetMetaData_Error_Exit;
	}

	xmlNode = XmlUtils::GetTag(xmlParser,(int8*)"Result");
	if(NULL != xmlNode){
		int8* res = XmlUtils::ExtractTagValue(xmlNode);
		if(res != NULL){
			xmlParser1.Parse(res);
			xmlNode = XmlUtils::GetTag(xmlParser1,(int8*)"container");
			if(NULL != xmlNode){
				aMetadataObj.Container = true;
				noAttr = XmlUtils::GetNumOfAttributes(xmlNode);
				for(attrCount=1; attrCount <= noAttr; attrCount++){
					xmlAttr = XmlUtils::GetAttributeAtIndex(xmlNode,attrCount);
					if(NULL != xmlAttr){
						if(0 == os_strcasecmp(xmlAttr->Name(),"childCount")){
							aMetadataObj.ChildCount = os_atoi(xmlAttr->Value());
						}else if(0 == os_strcasecmp(xmlAttr->Name(),"id")){
							os_strncpy(aMetadataObj.ID,xmlAttr->Value(),sizeof(aMetadataObj.ID));
							//aMetadataObj.ID = os_atoi(xmlAttr.AttribValue);
						}else if(0 == os_strcasecmp(xmlAttr->Name(),"parentID")){
							os_strncpy(aMetadataObj.ParentID,xmlAttr->Value(),sizeof(aMetadataObj.ParentID));
							//aMetadataObj.ParentID = os_atoi(xmlAttr.AttribValue);
						}
					}
				}
			}else{
				xmlNode = XmlUtils::GetTag(xmlParser1,(int8*)"item");
				if(NULL != xmlNode){
					aMetadataObj.Container = false;
					noAttr = XmlUtils::GetNumOfAttributes(xmlNode);
					for(attrCount=1; attrCount <= noAttr; attrCount++){
						xmlAttr = XmlUtils::GetAttributeAtIndex(xmlNode,attrCount);
						if(NULL != xmlAttr){
							if(0 == os_strcasecmp(xmlAttr->Name(),"childCount")){
								aMetadataObj.ChildCount = os_atoi(xmlAttr->Value());
							}else if(0 == os_strcasecmp(xmlAttr->Name(),"id")){
								os_strncpy(aMetadataObj.ID,xmlAttr->Value(),sizeof(aMetadataObj.ID));
								//aMetadataObj.ID = os_atoi(xmlAttr.AttribValue);
							}else if(0 == os_strcasecmp(xmlAttr->Name(),"parentID")){
								os_strncpy(aMetadataObj.ParentID,xmlAttr->Value(),sizeof(aMetadataObj.ParentID));
								//aMetadataObj.ParentID = os_atoi(xmlAttr.AttribValue);
							}
						}
					}
				}else{
					LOGE("cUPnPControlPoint::GetMetaData: this was neither Container nor item. Hence don't know what to do with it");
					status = FAILURE;
					goto GetMetaData_Error_Exit;
				}
			}

			/* Extract the Title.. */
			os_memset(tagValue,0,sizeof(tagValue));
			if(SUCCESS == XmlUtils::GetTagValue(xmlParser1,(int8*)"dc:title",tagValue)){
				os_strncpy(aMetadataObj.Title,tagValue,sizeof(aMetadataObj.Title));
			}

			/* Extract the Creator.. */
			os_memset(tagValue,0,sizeof(tagValue));
			if(SUCCESS == XmlUtils::GetTagValue(xmlParser1,(int8*)"dc:creator",tagValue)){
				os_strncpy(aMetadataObj.Creator,tagValue,sizeof(aMetadataObj.Creator));
			}

			/* Extract the Title.. */
			os_memset(tagValue,0,sizeof(tagValue));
			if(SUCCESS == XmlUtils::GetTagValue(xmlParser1,(int8*)"dc:title",tagValue)){
				os_strncpy(aMetadataObj.Title,tagValue,sizeof(aMetadataObj.Title));
			}
		}
	}

	status = SUCCESS;
GetMetaData_Error_Exit:
	if(postReqResultPtr){
		os_delete(postReqResultPtr);
	}
	return status;
}

int32 cUPnPControlPoint::GetChildCount(DeviceInfo aDevInfo,int8 *aObjId)
{
	//LOGD("cUPnPControlPoint::GetChildCount IN ObjectID = %s",aMetadataObj.ID);
	int8 *postReq = (int8*)os_malloc(HTTP_REQUEST_SIZE),
		 *browseDirectChildReq = (int8*)os_malloc(HTTP_REQUEST_SIZE),
		 *postRequestRecvHeader = (int8*)os_malloc(HTTP_HEADER_SIZE),
	     *postReqResultPtr = NULL,
	     tagValue[128] = {'\0'},controlURL[128] = {'\0'};

	ServiceInfo servInfo;
	TiXmlDocument xmlParser;
	int32 recvBytes = 0, contentLength = 0, servCount = 0, startIndex = 0, maxCount = 50;
	int32 status = SUCCESS, childCount = 0;

	/* Extract the service control URL for posing the request.. */
	for(servCount = 1; servCount <= aDevInfo.ServiceList->GetNumberOfElements(); servCount++){
		os_memset(&servInfo,0,sizeof(servInfo));
		if(SUCCESS == aDevInfo.ServiceList->GetElementAtIndex(servInfo,servCount)){
			if(CONTENT_DIRECTORY_SERVICE_ID == servInfo.ServiceID){
				break;
			}
		}
	}

	if(servCount > aDevInfo.ServiceList->GetNumberOfElements()){
		/* Even the server is found.. But this server is not supporting CDS.. Hence return failure to the app.. */
		LOGE("cUPnPControlPoint:GetChildCount server doesn't support CDS. Hence returning.");
		status = FAILURE;
		goto GetChildCount_Error_Exit;
	}

	/* Some stupid servers like PlayOn we need to add "/" infront of all requests. Otherwise it will through error.
	 * Don't know why they don't follow the guidelines. Stupid people and stupid servers...
	 */
	if(os_strlen(aDevInfo.DeviceStringInURL)){
		if(os_strcmp(aDevInfo.DeviceStringInURL,"/") == 0){
			os_strncpy(controlURL,aDevInfo.DeviceStringInURL,sizeof(controlURL));
			os_strcat(controlURL,servInfo.ControlURL);
		}else{
			os_strncpy(controlURL,servInfo.ControlURL,sizeof(controlURL));
		}
	}else{
		os_strncpy(controlURL,servInfo.ControlURL,sizeof(controlURL));
	}
	while(1){
		os_memset(postReq,0,HTTP_REQUEST_SIZE);
		os_snprintf(browseDirectChildReq,HTTP_REQUEST_SIZE,gUPnPCPActions[UPNP_CP_BROWSE_ACTION],aObjId,gUPnPCPTags[BROWSE_DIRECT_CHILDREN],"*",startIndex,maxCount,"");
		/* Create the POST request for sending to the server.. */
		os_snprintf(postReq,HTTP_REQUEST_SIZE,gUPnPCPActions[UPNP_CP_POST_REQ],controlURL,gUPnPCPTags[BROWSE_REQUEST],aDevInfo.DeviceIP,aDevInfo.DevicePortNo,os_strlen(browseDirectChildReq));
		LOGD("cUPnPControlPoint::GetChildCount Calling OpenServerConnectSocket For getting the children of [%s]",aObjId);
		if(-1 == cvActiveServerSockID){
			OpenServerConnectSocket(aDevInfo);
		}
		/* First send POST Request to the server. */
		if(os_strlen(postReq) != cSocket::Send(cvActiveServerSockID,postReq,os_strlen(postReq))){
			/* In case of Failure try again after opening the socket.. */
			OpenServerConnectSocket(aDevInfo);
			if(os_strlen(postReq) != cSocket::Send(cvActiveServerSockID,postReq,os_strlen(postReq))){
				LOGE("cUPnPControlPoint::GetChildCount: Error In Sending the postReqest");
				cSocket::CloseSocket(cvActiveServerSockID);
				cvActiveServerSockID = -1;
				status = FAILURE;
				goto GetChildCount_Error_Exit;
			}
		}

		/* Now start sending the xml request for Metadata.. */
		if(os_strlen(browseDirectChildReq) != cSocket::Send(cvActiveServerSockID,browseDirectChildReq,os_strlen(browseDirectChildReq))){
			LOGE("cUPnPControlPoint::GetChildCount: Error In Sending the postReqest body");
			/* Some problem in sending the post request.. */
			cSocket::CloseSocket(cvActiveServerSockID);
			cvActiveServerSockID = -1;
			status = FAILURE;
			goto GetChildCount_Error_Exit;
		}

		os_memset(postRequestRecvHeader,0,HTTP_REQUEST_SIZE);
		/* Now receive the response header for the POST request sent above.. */
		if(FAILURE == cHttpUtils::ReceiveHeader(cvActiveServerSockID,postRequestRecvHeader,HTTP_REQUEST_SIZE)){
			LOGE("cUPnPControlPoint::GetChildCount: Recv Header Error");
			status = FAILURE;
			goto GetChildCount_Error_Exit;
		}

		/* Now Parse the response header and if it is fine then go ahead to receive the complete deviceinfo.xml file.. */
		if(NULL == os_strcasestr(postRequestRecvHeader,"HTTP/1.1 200 OK")){
			LOGE("cUPnPControlPoint::GetChildCount: HTTP Error code Error");
			status = FAILURE;
			goto GetChildCount_Error_Exit;
		}

		/* As of now we don't support chunked transfer. When we support we can enable it.. */
		if((NULL == os_strcasestr(postRequestRecvHeader,"CONTENT-LENGTH")) && (NULL == os_strstr(postRequestRecvHeader,"chunked"))){
			LOGE("cUPnPControlPoint::GetChildCount: No Content Length Error");
			status = FAILURE;
			goto GetChildCount_Error_Exit;
		}

		/* Now extract the content length */
		contentLength = cHttpUtils::ExtractContentLength(postRequestRecvHeader);
		if(FAILURE == contentLength){
			LOGE("cUPnPControlPoint::GetChildCount: Extracting Content Length Error");
			status = FAILURE;
			goto GetChildCount_Error_Exit;
		}

		postReqResultPtr = (int8*)os_malloc(contentLength+1);
		if(NULL == postReqResultPtr){
			LOGE("cUPnPControlPoint::GetChildCount: Memory Allocation Error");
			status = FAILURE;
			goto GetChildCount_Error_Exit;
		}

		recvBytes = cSocket::Receive(cvActiveServerSockID,postReqResultPtr,contentLength);
		if(contentLength != recvBytes){
			LOGE("cUPnPControlPoint::GetChildCount: Recv Error. Received Bytes [%d] ContentLength [%d]",recvBytes,contentLength);
			status = FAILURE;
			goto GetChildCount_Error_Exit;
		}
		//LOG_CONSOLE("POST Request result: \n %s",postReqResultPtr);

		if(CONNECTION_CLOSE == cHttpUtils::GetConnectionType(postRequestRecvHeader)){
			cSocket::CloseSocket(cvActiveServerSockID);
			cvActiveServerSockID = -1;
		}

		/* Now we got the response. Need to parse and extract the Child information.. */
		xmlParser.Parse(postReqResultPtr);

		os_memset(tagValue,0,sizeof(tagValue));
		if(SUCCESS == XmlUtils::GetTagValue(xmlParser,(int8*)"NumberReturned",tagValue)){ /* If we don't have ths tag means then there is something wrong somewhere.. Find out.. */
			status = os_atoi(tagValue);
		}else{
			LOGE("cUPnPControlPoint::GetChildCount: Failed to get the Tag Value for \"NumberReturned\"");
			status = FAILURE;
			goto GetChildCount_Error_Exit;
		}
		childCount += status;
		startIndex += maxCount;
		if(status < maxCount){
			break;
		}
		xmlParser.Clear();
		xmlParser.~TiXmlDocument();
		if(postReqResultPtr){
			os_delete(postReqResultPtr);
			postReqResultPtr = NULL;
		}
	}

	GetChildCount_Error_Exit:
		if(postReq){
			os_delete(postReq);
		}
		if(browseDirectChildReq){
			os_delete(browseDirectChildReq);
		}
		if(postRequestRecvHeader){
			os_delete(postRequestRecvHeader);
		}
		if(postReqResultPtr){
			os_delete(postReqResultPtr);
		}
		if(childCount < 0){
			childCount = 0;
		}
		return childCount;
}

ReturnStatus cUPnPControlPoint::GetDirectChilds(DeviceInfo aDevInfo, MetaDataObj &aMetadataObj,int32 startIndex, int32 limit)
{
	//LOGD("cUPnPControlPoint::GetDirectChilds IN ObjectID = %s",aMetadataObj.ID);
	int8 *postReq = (int8*)os_malloc(HTTP_REQUEST_SIZE),
	     *browseDirectChildReq = (int8*)os_malloc(HTTP_REQUEST_SIZE),
	     *postRequestRecvHeader = (int8*)os_malloc(HTTP_HEADER_SIZE),
	     *postReqResultPtr = NULL,
	     tagValue[128] = {'\0'},controlURL[128]={'\0'};

	ServiceInfo servInfo;
	TiXmlDocument xmlParser, xParser;
	TiXmlNode* xmlTag;
	TiXmlAttribute* xmlAttr;
	MetaDataObj tempObj;
	MediaObj tempMediaObj;
	int32 recvBytes = 0, contentLength = 0, servCount = 0, noAttr = 0, attrCount = 0, reqCount = 0, /*returnCount = 0,*/ numberItems = 0,itemCount = 0;
	ReturnStatus status = SUCCESS;

	os_memset(postReq,'\0',HTTP_REQUEST_SIZE);
	os_memset(browseDirectChildReq,'\0',HTTP_REQUEST_SIZE);
	os_memset(postRequestRecvHeader,'\0',HTTP_REQUEST_SIZE);

	/* Extract the service control URL for posing the request.. */
	for(servCount = 1; servCount <= aDevInfo.ServiceList->GetNumberOfElements(); servCount++){
		os_memset(&servInfo,0,sizeof(servInfo));
		if(SUCCESS == aDevInfo.ServiceList->GetElementAtIndex(servInfo,servCount)){
			if(CONTENT_DIRECTORY_SERVICE_ID == servInfo.ServiceID){
				break;
			}
		}
	}

	if(servCount > aDevInfo.ServiceList->GetNumberOfElements()){
		/* Even the server is found.. But this server is not supporting CDS.. Hence return failure to the app.. */
		LOGE("cUPnPControlPoint:GetDirectChilds server doesn't support CDS. Hence returning.");
		status = FAILURE;
		goto GetDirectChilds_Error_Exit;
	}
	if(-1 == aMetadataObj.ChildCount){
		aMetadataObj.ChildCount = GetChildCount(aDevInfo,aMetadataObj.ID);
	}
	/* Request the objects from the server in chunks... */
	if(startIndex < aMetadataObj.ChildCount){
		reqCount = limit;

		/* Some stupid servers like PlayOn we need to add "/" infront of all requests. Otherwise it will through error.
		 * Don't know why they don't follow the guidelines. Stupid people and stupid servers...
		 */
		if(os_strlen(aDevInfo.DeviceStringInURL)){
			if(os_strcmp(aDevInfo.DeviceStringInURL,"/") == 0){
				os_strncpy(controlURL,aDevInfo.DeviceStringInURL,sizeof(controlURL));
				os_strcat(controlURL,servInfo.ControlURL);
			}else{
				os_strncpy(controlURL,servInfo.ControlURL,sizeof(controlURL));
			}
		}else{
			os_strncpy(controlURL,servInfo.ControlURL,sizeof(controlURL));
		}

		/* Create the XML request for requesting the direct children from the server.. */
		os_snprintf(browseDirectChildReq,HTTP_REQUEST_SIZE,gUPnPCPActions[UPNP_CP_BROWSE_ACTION],aMetadataObj.ID,gUPnPCPTags[BROWSE_DIRECT_CHILDREN],"*",startIndex,reqCount,"");
		/* Create the POST request for sending to the server.. */
		os_snprintf(postReq,HTTP_REQUEST_SIZE,gUPnPCPActions[UPNP_CP_POST_REQ],controlURL,gUPnPCPTags[BROWSE_REQUEST],aDevInfo.DeviceIP,aDevInfo.DevicePortNo,os_strlen(browseDirectChildReq));
		//LOGD("cUPnPControlPoint::GetDirectChilds Calling OpenServerConnectSocket For getting the children of [%s]",aMetadataObj.ID);
		if(-1 == cvActiveServerSockID){
			OpenServerConnectSocket(aDevInfo);
		}
		/* First send POST Request to the server. */
		if(os_strlen(postReq) != cSocket::Send(cvActiveServerSockID,postReq,os_strlen(postReq))){
			/* In Case of Failure try opening the socket again. */
			OpenServerConnectSocket(aDevInfo);
			if(os_strlen(postReq) != cSocket::Send(cvActiveServerSockID,postReq,os_strlen(postReq))){
				LOGE("cUPnPControlPoint::GetDirectChilds: Error While sending the POST request");
				cSocket::CloseSocket(cvActiveServerSockID);
				cvActiveServerSockID = -1;
				status = FAILURE;
				goto GetDirectChilds_Error_Exit;
			}
		}

		/* Now start sending the xml request for Metadata.. */
		if(os_strlen(browseDirectChildReq) != cSocket::Send(cvActiveServerSockID,browseDirectChildReq,os_strlen(browseDirectChildReq))){
			LOGE("cUPnPControlPoint::GetDirectChilds: Error While sending the POST request Body");
			/* Some problem in sending the post request.. */
			cSocket::CloseSocket(cvActiveServerSockID);
			cvActiveServerSockID = -1;
			status = FAILURE;
			goto GetDirectChilds_Error_Exit;
		}

		/* Now receive the response header for the POST request sent above.. */
		if(FAILURE == cHttpUtils::ReceiveHeader(cvActiveServerSockID,postRequestRecvHeader,HTTP_REQUEST_SIZE)){
			LOGE("cUPnPControlPoint::GetDirectChilds: Recv Header Error");
			status = FAILURE;
			goto GetDirectChilds_Error_Exit;
		}

		/* Now Parse the response header and if it is fine then go ahead to receive the complete deviceinfo.xml file.. */
		if(NULL == os_strcasestr(postRequestRecvHeader,"HTTP/1.1 200 OK")){
			LOGE("cUPnPControlPoint::GetDirectChilds: HTTP Error code Error");
			status = FAILURE;
			goto GetDirectChilds_Error_Exit;
		}

		/* As of now we don't support chunked transfer. When we support we can enable it.. */
		if((NULL == os_strcasestr(postRequestRecvHeader,"CONTENT-LENGTH")) && (NULL == os_strstr(postRequestRecvHeader,"chunked"))){
			LOGE("cUPnPControlPoint::GetDirectChilds: No Content Length Error");
			status = FAILURE;
			goto GetDirectChilds_Error_Exit;
		}

		/* Now extract the content length */
		contentLength = cHttpUtils::ExtractContentLength(postRequestRecvHeader);
		if(FAILURE == contentLength){
			LOGE("cUPnPControlPoint::GetDirectChilds: Extracting Content Length Error");
			status = FAILURE;
			goto GetDirectChilds_Error_Exit;
		}

		postReqResultPtr = (int8*)os_malloc(contentLength+1);
		if(NULL == postReqResultPtr){
			LOGE("cUPnPControlPoint::GetDirectChilds: Memory Allocation Error");
			status = FAILURE;
			goto GetDirectChilds_Error_Exit;
		}

		recvBytes = cSocket::Receive(cvActiveServerSockID,postReqResultPtr,contentLength);
		if(contentLength != recvBytes){
			LOGE("cUPnPControlPoint::GetDirectChilds: Recv Error. Received Bytes [%d] ContentLength [%d]",recvBytes,contentLength);
			status = FAILURE;
			goto GetDirectChilds_Error_Exit;
		}
		//LOG_CONSOLE("POST Request result: \n %s",postReqResultPtr);

		if(CONNECTION_CLOSE == cHttpUtils::GetConnectionType(postRequestRecvHeader)){
			cSocket::CloseSocket(cvActiveServerSockID);
			cvActiveServerSockID = -1;
		}

		/* Now we got the response. Need to parse and extract the Child information.. */
		xmlParser.Parse(postReqResultPtr);

		/*os_memset(tagValue,0,sizeof(tagValue));
		if(SUCCESS == XmlUtils::GetTagValue(xmlParser,(int8*)"NumberReturned",tagValue)){
			//returnCount = os_atoi(tagValue);
		}else{
			LOGE("cUPnPControlPoint::GetDirectChilds: Failed to get the Tag Value \"NumberReturned\"");
			status = FAILURE;
			goto GetDirectChilds_Error_Exit;
		}*/
		TiXmlNode *result = XmlUtils::GetTag(xmlParser,(int8*)"Result");
		if(NULL == result){
			LOGE("cUPnPControlPoint::GetDirectChilds: Failed to get the Tag \"Result\"");
			status = FAILURE;
			goto GetDirectChilds_Error_Exit;
		}
		char *resultPtr = XmlUtils::ExtractTagValue(result);
		if(NULL == resultPtr){
			LOGE("cUPnPControlPoint::GetDirectChilds: Failed to get the Tag Value for \"Result\"");
			status = FAILURE;
			goto GetDirectChilds_Error_Exit;
		}
		xParser.Parse(resultPtr);
		xmlParser.Clear();
		xmlParser.~TiXmlDocument();
		/* Increment the childCount variable as per the number of items we got.. */
		xmlTag = XmlUtils::GetTag(xParser,(int8*)"DIDL-Lite");
		if(NULL == xmlTag){
			LOGE("cUPnPControlPoint::GetDirectChilds: Failed to get the Tag \"DIDL-Lite\"");
			status = FAILURE;
			goto GetDirectChilds_Error_Exit;
		}

		numberItems = XmlUtils::GetNumOfChilds(xmlTag);
		//LOGD("cUPnPControlPoint::GetDirectChilds Number Of subtabs = %d",numberItems);
		if(0 == numberItems){
			/* Ideally we should not hit this scenario of server saying the childcound and when we ask for childs returning nothing.
			 * So for now treat this as success only and continue... Remember this is the issue with some servers.. not US.. :)
			 */
			aMetadataObj.ObjectsAlreadyRetrieved = true;
			status = SUCCESS;
			goto GetDirectChilds_Error_Exit;
		}
		for(itemCount = 1; itemCount <= numberItems; itemCount++){
			TiXmlNode* tag = XmlUtils::GetSubTagAtIndex(xmlTag,itemCount);
			if(NULL != tag){
				os_memset(&tempObj,0,sizeof(tempObj));
				os_memset(&tempMediaObj,0,sizeof(tempMediaObj));
				tempMediaObj.DeviceID = cvActiveServerID;
				tempObj.ParentPtr = &aMetadataObj;
				os_strncpy(tempMediaObj.DeviceName,cvActiveServerName,sizeof(tempMediaObj.DeviceName));
				if(os_strcasecmp(tag->Value(),(int8*)"container") == 0){
					tempObj.Container = true;
					tempObj.ObjectID = cvObjectID++;
					tempObj.ChildCount = -1;
					noAttr = XmlUtils::GetNumOfAttributes(tag);
					for(attrCount=1; attrCount <= noAttr; attrCount++){
						xmlAttr = XmlUtils::GetAttributeAtIndex(tag,attrCount);
						if(NULL != xmlAttr){
							if(0 == os_strcasecmp(xmlAttr->Name(),"childCount")){
								tempObj.ChildCount = os_atoi(xmlAttr->Value());
							}else if(0 == os_strcasecmp(xmlAttr->Name(),"id")){
								os_strncpy(tempObj.ID,xmlAttr->Value(),sizeof(tempObj.ID));
								os_strncpy(tempMediaObj.ID,xmlAttr->Value(),sizeof(tempMediaObj.ID));
								//tempObj.ID = os_atoi(xmlAttr.AttribValue);
							}else if(0 == os_strcasecmp(xmlAttr->Name(),"parentID")){
								os_strncpy(tempObj.ParentID,xmlAttr->Value(),sizeof(tempObj.ParentID));
								//tempObj.ParentID = os_atoi(xmlAttr.AttribValue);
							}
						}
					}
				}else if(os_strcasecmp(tag->Value(),(int8*)"item") == 0){
					tempObj.Container = false;
					tempObj.ObjectID = cvObjectID++;
					noAttr = XmlUtils::GetNumOfAttributes(tag);
					for(attrCount=1; attrCount <= noAttr; attrCount++){
						xmlAttr = XmlUtils::GetAttributeAtIndex(tag,attrCount);
						if(NULL != xmlAttr){
							if(0 == os_strcasecmp(xmlAttr->Name(),"childCount")){
								tempObj.ChildCount = os_atoi(xmlAttr->Value());
							}else if(0 == os_strcasecmp(xmlAttr->Name(),"id")){
								os_strncpy(tempObj.ID,xmlAttr->Value(),sizeof(tempObj.ID));
								os_strncpy(tempMediaObj.ID,xmlAttr->Value(),sizeof(tempMediaObj.ID));
								//tempObj.ID = os_atoi(xmlAttr.AttribValue);
							}else if(0 == os_strcasecmp(xmlAttr->Name(),"parentID")){
								os_strncpy(tempObj.ParentID,xmlAttr->Value(),sizeof(tempObj.ParentID));
								//tempObj.ParentID = os_atoi(xmlAttr.AttribValue);
							}
						}
					}
				}else{
					continue;
				}

				/* Extract the Title.. */
				os_memset(tagValue,0,sizeof(tagValue));
				if(SUCCESS == XmlUtils::GetTagValue(tag,(int8*)"dc:title",tagValue)){
					os_strncpy(tempObj.Title,tagValue,sizeof(tempObj.Title));
					os_strncpy(tempMediaObj.Title,tagValue,sizeof(tempMediaObj.Title));
				}

				/* Extract the Creator.. */
				os_memset(tagValue,0,sizeof(tagValue));
				if(SUCCESS == XmlUtils::GetTagValue(tag,(int8*)"dc:creator",tagValue)){
					os_strncpy(tempObj.Creator,tagValue,sizeof(tempObj.Creator));
					os_strncpy(tempMediaObj.Creator,tagValue,sizeof(tempMediaObj.Creator));
				}

				/* Extract the Artist.. */
				os_memset(tagValue,0,sizeof(tagValue));
				if(SUCCESS == XmlUtils::GetTagValue(tag,(int8*)"dc:artist",tagValue)){
					os_strncpy(tempObj.Artist,tagValue,sizeof(tempObj.Artist));
					os_strncpy(tempMediaObj.Artist,tagValue,sizeof(tempMediaObj.Artist));
				}

				/* Extract the Album.. */
				os_memset(tagValue,0,sizeof(tagValue));
				if(SUCCESS == XmlUtils::GetTagValue(tag,(int8*)"dc:album",tagValue)){
					os_strncpy(tempObj.Album,tagValue,sizeof(tempObj.Album));
					os_strncpy(tempMediaObj.Album,tagValue,sizeof(tempMediaObj.Album));
				}

				/* Extract the Genre.. */
				os_memset(tagValue,0,sizeof(tagValue));
				if(SUCCESS == XmlUtils::GetTagValue(tag,(int8*)"dc:genre",tagValue)){
					os_strncpy(tempObj.Genre,tagValue,sizeof(tempObj.Genre));
					os_strncpy(tempMediaObj.Genre,tagValue,sizeof(tempMediaObj.Genre));
				}

				/* Now Parse the res element.. */
				TiXmlNode* tag2 = XmlUtils::GetTag(tag,(int8*)"res");
				if(NULL != tag2){
					noAttr = XmlUtils::GetNumOfAttributes(tag2);
					for(attrCount=1; attrCount <= noAttr; attrCount++){
						xmlAttr = XmlUtils::GetAttributeAtIndex(tag2,attrCount);
						if(NULL != xmlAttr){
							if(0 == os_strcasecmp(xmlAttr->Name(),"size")){
								tempObj.Size = os_atoi(xmlAttr->Value());
								tempMediaObj.Size = os_atoi(xmlAttr->Value());
							}else if(0 == os_strcasecmp(xmlAttr->Name(),"protocolInfo")){
								os_strncpy(tempObj.ProtocolInfo,xmlAttr->Value(),sizeof(tempObj.ProtocolInfo));
								os_strncpy(tempMediaObj.ProtocolInfo,xmlAttr->Value(),sizeof(tempMediaObj.ProtocolInfo));
							}
							else if(0 == os_strcasecmp(xmlAttr->Name(),"duration")){
								int8 temp[16] = {'\0'},*tempPtr = NULL;
								int32 sec = 0,min = 0, hr = 0;
								os_strncpy(temp,xmlAttr->Value(),sizeof(temp));
								//LOGD("cUPnPControlPoint::GetDirectChilds: Duration String = %s",temp);
								tempPtr = (int8*)os_strrstr(temp,(int8*)":");
								if(tempPtr){
									//LOGD("cUPnPControlPoint::GetDirectChilds: msec string = %s",tempPtr+1);
									sec = os_atoi(tempPtr+1);
									*tempPtr = '\0';
									tempPtr = (int8*)os_strrstr(temp,(int8*)":");
									if(tempPtr){
										//LOGD("cUPnPControlPoint::GetDirectChilds: sec string = %s",tempPtr+1);
										min = os_atoi(tempPtr+1);
										*tempPtr = '\0';
										//LOGD("cUPnPControlPoint::GetDirectChilds: hr string = %s",temp);
										hr = os_atoi(temp);
									}
								}
								//LOGD("cUPnPControlPoint::GetDirectChilds: hr = %d, min = %d, sec = %d",hr,min,sec);
								tempObj.Duration = (((hr*60*60) + (min*60) + sec) * 1000);
								tempMediaObj.Duration = tempObj.Duration;
								//LOGD("cUPnPControlPoint::GetDirectChilds: Final Converted Duration = %d",tempObj.Duration);
							}
						}
					}
				}

				/* Extract the URL.. */
				os_memset(tagValue,0,sizeof(tagValue));
				if(NULL != tag2 && SUCCESS == XmlUtils::ExtractTagValue(tag2,tagValue)){
					os_strncpy(tempObj.URL,tagValue,sizeof(tempObj.URL));
					os_strncpy(tempMediaObj.URL,tagValue,sizeof(tempMediaObj.URL));
				}

				/* Fill the depth of this item.. */
				tempObj.FolderDepth = aMetadataObj.FolderDepth + 1;
				
				TiXmlNode* tag3 = XmlUtils::GetTag(tag,(int8*)"upnp:albumArtURI");
				if(NULL != tag3){
					os_memset(tagValue,0,sizeof(tagValue));
					if(SUCCESS == XmlUtils::ExtractTagValue(tag3,tagValue)){
						os_strncpy(tempObj.ThumbnailURL,tagValue,sizeof(tempObj.ThumbnailURL));
						os_strncpy(tempMediaObj.ThumbnailURL,tagValue,sizeof(tempMediaObj.ThumbnailURL));
					}
				}
				/* Adding the to the Songs/Videos/Pictures list only for the items. No need to consider Containers. */
				if(false == tempObj.Container && 0 != os_strlen(tempObj.ProtocolInfo)){
					if(os_strcasestr(tempObj.ProtocolInfo,"audio/")){
						tempObj.ContainItemsType = MUSIC;
						cvSongsList.Add(tempMediaObj);
					}else if(os_strcasestr(tempObj.ProtocolInfo,"video/")){
						tempObj.ContainItemsType = VIDEO;
						cvVideosList.Add(tempMediaObj);
					}else if(os_strcasestr(tempObj.ProtocolInfo,"image/")){
						tempObj.ContainItemsType = PICTURES;
						/* Now Here try to extract the Thumbnail with smaller size.. */
						if(os_strlen(tempObj.ThumbnailURL) <= 0){
							if(NULL != tag){
								int32 size = 0;
								int8 thumbnailURL[128] = {'\0'};
								/* In case if album art uri is not present then try searching in res elements for suitable small image.. */
								int resCount = 1;
								TiXmlNode *tag4 = NULL;
								do{
									tag4 = XmlUtils::GetRepetetiveTagAtIndex(tag,(int8*)"res",resCount);
									noAttr = XmlUtils::GetNumOfAttributes(tag4);
									for(attrCount=1; attrCount <= noAttr; attrCount++){
										TiXmlAttribute* xAttr = XmlUtils::GetAttributeAtIndex(tag4,attrCount);
										if(NULL != xAttr){
											if(0 == os_strcasecmp(xAttr->Name(),"size")){
												int32 temp = os_atoi(xAttr->Value());
												if(size == 0 || size > temp){
													size = temp;
													os_memset(thumbnailURL,'\0',sizeof(thumbnailURL));
													XmlUtils::ExtractTagValue(tag4,thumbnailURL);
												}
											}
										}
									}
									resCount++;
								}while(tag4 != NULL);
								if(os_strlen(thumbnailURL) > 0 && size > 0){
									os_strncpy(tempObj.ThumbnailURL,thumbnailURL,sizeof(tempObj.ThumbnailURL));
									os_strncpy(tempMediaObj.ThumbnailURL,thumbnailURL,sizeof(tempMediaObj.ThumbnailURL));
								}
							}
						}
						cvPicturesList.Add(tempMediaObj);
					}
				}	

				/* After successfully filling all the required fields of the item just append it to the list.. */
				//LOG_CONSOLE("cUPnPControlPoint::GetDirectChilds Adding Object [%s] with ID [%s]",tempObj.Title,tempObj.ID);
				aMetadataObj.ChildList->Add(tempObj);
			}
		}
		if(postReqResultPtr){
			os_free(postReqResultPtr);
			postReqResultPtr = NULL;
		}
	}
	if(startIndex+limit >= aMetadataObj.ChildCount){
		aMetadataObj.ObjectsAlreadyRetrieved = true;
	}

	status = SUCCESS;
GetDirectChilds_Error_Exit:
	if(postReq){
		os_delete(postReq);
	}
	if(browseDirectChildReq){
		os_delete(browseDirectChildReq);
	}
	if(postRequestRecvHeader){
		os_delete(postRequestRecvHeader);
	}
	if(postReqResultPtr){
		os_delete(postReqResultPtr);
	}
	//xmlParser.Clear();
	xParser.Clear();
	xParser.~TiXmlDocument();
	return status;
}

ReturnStatus cUPnPControlPoint::ExploreServer(DeviceInfo aDevInfo, MetaDataObj &aMetadataObj)
{
	//LOGD("cUPnPControlPoint::ExploreServer IN");
	MetaDataObj tempObj;
	int32 count = 0;
	if(1 == aMetadataObj.Container){
		aMetadataObj.ChildList = os_new(cLinkedList<struct metaDataObj>,());
		if(NULL == aMetadataObj.ChildList){
			return FAILURE;
		}
		//LOGD("cUPnPControlPoint::ExploreServer Calling GetDirectChilds with ObjectId = %s",aMetadataObj.ID);
		while(count < aMetadataObj.ChildCount){
			GetDirectChilds(aDevInfo,aMetadataObj,count,50);
			count += 50;
		}
		//LOG_CONSOLE("ExploreServer: Number of childs for [%s]  is [%d] ",aMetadataObj.Title,aMetadataObj.ChildList->GetNumberOfElements());
		cLinkedList<struct metaDataObj> *childlist = aMetadataObj.ChildList;
		cLinkedList<struct metaDataObj> *finalList = os_new(cLinkedList<struct metaDataObj>,());
		while(childlist->GetNumberOfElements()){
			os_memset(&tempObj,0,sizeof(tempObj));
			childlist->RemoveAtStart(tempObj);
			if((1 == tempObj.Container)){
				//LOGD("cUPnPControlPoint::ExploreServer Calling again ExploreServer with ObjectId = %s",aMetadataObj.ID);
				if(SUCCESS == ExploreServer(aDevInfo,tempObj)){
					/* We got the data from server.. */
				}
			}else{
				if(MUSIC == tempObj.ContainItemsType){
					//LOG_CONSOLE("Calling UpdateContentTypeToParentFolders to Update for MUSIC for the object [%s] with Parent folde [%s]",tempObj.Title,aMetadataObj.Title);
					UpdateCotentTypeToParentFolders(aMetadataObj,MUSIC);
				}else if(VIDEO == tempObj.ContainItemsType){
					//LOG_CONSOLE("Calling UpdateContentTypeToParentFolders to Update for VIDEO for the object [%s] with Parent folde [%s]",tempObj.Title,aMetadataObj.Title);
					UpdateCotentTypeToParentFolders(aMetadataObj,VIDEO);
				}else if(PICTURES == tempObj.ContainItemsType){
					//LOG_CONSOLE("Calling UpdateContentTypeToParentFolders to Update for PICTURES for the object [%s] with Parent folde [%s]",tempObj.Title,aMetadataObj.Title);
					UpdateCotentTypeToParentFolders(aMetadataObj,PICTURES);
				}
			}
			finalList->Add(tempObj);
		}
		aMetadataObj.ChildList->DestroyList();
		if(aMetadataObj.ChildList){
			os_delete(aMetadataObj.ChildList);
		}
		aMetadataObj.ChildList = finalList;
	}
	//LOGD("cUPnPControlPoint::ExploreServer OUT");
	return SUCCESS;
}

void
cUPnPControlPoint::UpdateCotentTypeToParentFolders(MetaDataObj &aMetadataObj, ContentType type)
{
	MetaDataObj *tempObj = &aMetadataObj;
	ContentType ctype = (ContentType)(tempObj->ContainItemsType | type);
	if(tempObj->ContainItemsType == type){
		return;
	}
	while(tempObj->ParentPtr != NULL && tempObj->ContainItemsType != ctype){
		//LOG_CONSOLE("Calling Updating the ContainItemsType of [%s] to [%d]",tempObj->Title,ctype);
		tempObj->ContainItemsType = ctype;
		tempObj = tempObj->ParentPtr;
		ctype = (ContentType)(tempObj->ContainItemsType | type);
		if(tempObj->ParentPtr == NULL){
			ctype = (ContentType)(tempObj->ContainItemsType | type);
			//LOG_CONSOLE("Calling Updating the ContainItemsType of [%s] to [%d]",tempObj->Title,ctype);
			tempObj->ContainItemsType = ctype;
		}
	}
}

DeviceInfo cUPnPControlPoint::RetrieveDeviceInfo(int32 aServerID)
{
	int32 devCount = 0;
	DeviceInfo devInfo;
	os_memset(&devInfo,0,sizeof(DeviceInfo));

	for(devCount = 1; devCount <= cvMediaServerList.GetNumberOfElements(); devCount++){
		os_memset(&devInfo,0,sizeof(devInfo));
		if(SUCCESS == cvMediaServerList.GetElementAtIndex(devInfo,devCount)){
			if(devInfo.deviceID == aServerID){
				/* Server found in the list... */
				break;
			}
		}
	}

	return devInfo;
}

SOCKET cUPnPControlPoint::OpenServerConnectSocket(DeviceInfo aDevInfo)
{
	cSocket socket;
	//LOGD("cUPnPControlPoint::OpenServerConnectSocket IN");
	if(-1 != cvActiveServerSockID){
		cSocket::CloseSocket(cvActiveServerSockID);
		cvActiveServerSockID = -1;
	}

	/* Initialize the Socket Params for creating the corresponding socket.. */
	socket.InitializeParams(aDevInfo.DeviceIP,aDevInfo.DevicePortNo,this);
	cvActiveServerSockID = socket.CreateTCPClientSocket();

	return cvActiveServerSockID;
}

ReturnStatus cUPnPControlPoint::SelectServer(int32 aServerID, flag aCacheData)
{
	//LOGD("cUPnPControlPoint::SelectServer IN");
	ClearServerSelection();

	DeviceInfo devInfo;
	cvCacheServerData = aCacheData;

	/* Check the Active Server ID and if it matches with the incomming serverID then no need to do any thing. Just return from here itself.. */
	if(aServerID == cvActiveServerID){
		return SUCCESS;
	}else{
		cvActiveServerID = aServerID;
	}

	devInfo = RetrieveDeviceInfo(aServerID);
	os_memset(&cvActiveDevInfo,0,sizeof(cvActiveDevInfo));
	os_memcpy(&cvActiveDevInfo,&devInfo,sizeof(cvActiveDevInfo));
	os_strncpy(cvActiveServerName,devInfo.DeviceName,sizeof(cvActiveServerName));


	os_memset(&cvServerObjects,0,sizeof(cvServerObjects));
	cvObjectID = 0;

	/* First here get the Metadata of the root folder. SO that we will come to know how many sub folders are present.. */
	if(SUCCESS != GetMetaData(devInfo,0,cvServerObjects)){
		/* Some server may not support GetMetaData. Try getting the child count and if still fails then return failure. */
		int8 objID[16] = {'\0'};
		os_snprintf(objID,sizeof(objID),"%d",0);
		int childCount = GetChildCount(devInfo,objID);
		if(SUCCESS != childCount){
			LOGE("cUPnPControlPoint::SelectServer Failed to get the Child Count as well as GetMetadata. Hence Don't know how to proceed");
			return FAILURE;
		}
		cvServerObjects.ChildCount = childCount;
	}
	/* Initially give the Container ID of the Root to be 0 */
	cvServerObjects.ObjectID = cvObjectID++;

	/* Clear the existing Song/Video/Picture lists. */
	cvSongsList.DestroyList();
	cvVideosList.DestroyList();
	cvPicturesList.DestroyList();

	if(true == cvCacheServerData){ /* If the app ask us to cache the server data then cache it.. otherwise don't do it..*/
		/* Now we came to know about the root folder. Start exploring further.. */
		if(SUCCESS != ExploreServer(devInfo,cvServerObjects)){
			LOGD("cUPnPControlPoint::SelectServer failed to explore the server.");
			return FAILURE;
		}
	}
	//LOGD("cUPnPControlPoint::SelectServer OUT returning SUCCESS");
	return SUCCESS;
}

void cUPnPControlPoint::ClearServerSelection()
{
	cvActiveServerID = -1;
	DelelteServerCache(cvServerObjects);
	cvObjectID = 0;
	cvCacheServerData = false;
	cvActiveServerSockID = -1;
	os_memset(&cvActiveDevInfo,0,sizeof(cvActiveDevInfo));
}

void
cUPnPControlPoint::CopyObjectsToUser(MetaDataObj &aMetadataObj, cLinkedList<MetaDataObj> &aObjList,int32 startIndex, int32 limit,ContentType type)
{
	int32 count = 0;
	MetaDataObj tempObj1;
	int32 num=0;
	if(true == aMetadataObj.Container){
		for(count=1; count <= aMetadataObj.ChildList->GetNumberOfElements(); count++){
			os_memset(&tempObj1,0,sizeof(tempObj1));
			if(SUCCESS == aMetadataObj.ChildList->GetElementAtIndex(tempObj1,count)){
				if(tempObj1.Container == true && (type != tempObj1.ContainItemsType && false != cvCacheServerData)){
					continue;
				}else if(tempObj1.Container == true){
					if(MUSIC != type){
						if(os_strcasecmp(tempObj1.Title,"Music") == 0){
							LOGD("cUPnPControlPoint::CopyObjectsToUser: Ignoring the folder as it matched with \"Music\" Name");
							continue;
						}else if(os_strcasecmp(tempObj1.Title,"Audio") == 0){
							LOGD("cUPnPControlPoint::CopyObjectsToUser: Ignoring the folder as it matched with \"Audio\" Name");
							continue;
						}
					}

					if(VIDEO != type){
						if(os_strcasecmp(tempObj1.Title,"Video") == 0){
							LOGD("cUPnPControlPoint::CopyObjectsToUser: Ignoring the folder as it matched with \"Video\" Name");
							continue;
						}else if(os_strcasecmp(tempObj1.Title,"Videos") == 0){
							LOGD("cUPnPControlPoint::CopyObjectsToUser: Ignoring the folder as it matched with \"Videos\" Name");
							continue;
						}
					}

					if(PICTURES != type){
						if(os_strcasecmp(tempObj1.Title,"Image") == 0){
							LOGD("cUPnPControlPoint::CopyObjectsToUser: Ignoring the folder as it matched with \"Image\" Name");
							continue;
						}else if(os_strcasecmp(tempObj1.Title,"Images") == 0){
							LOGD("cUPnPControlPoint::CopyObjectsToUser: Ignoring the folder as it matched with \"Images\" Name");
							continue;
						}else if(os_strcasecmp(tempObj1.Title,"Picture") == 0){
							LOGD("cUPnPControlPoint::CopyObjectsToUser: Ignoring the folder as it matched with \"Picture\" Name");
							continue;
						}else if(os_strcasecmp(tempObj1.Title,"Pictures") == 0){
							LOGD("cUPnPControlPoint::CopyObjectsToUser: Ignoring the folder as it matched with \"Pictures\" Name");
							continue;
						}else if(os_strcasecmp(tempObj1.Title,"Photo") == 0){
							LOGD("cUPnPControlPoint::CopyObjectsToUser: Ignoring the folder as it matched with \"Photo\" Name");
							continue;
						}else if(os_strcasecmp(tempObj1.Title,"Photos") == 0){
							LOGD("cUPnPControlPoint::CopyObjectsToUser: Ignoring the folder as it matched with \"Photos\" Name");
							continue;
						}
					}

				}else if(tempObj1.Container == false && type != tempObj1.ContainItemsType){
					continue;
				}
				if(num<startIndex){
					num++;
					continue;
				}
				if(num > startIndex+limit){
					return;
				}
				num++;
				aObjList.Add(tempObj1);
			}
		}
	}
}

cLinkedList<MediaObj>*
cUPnPControlPoint::GetAllSongs(int32 aServerID)
{
	return &cvSongsList;
}

cLinkedList<MediaObj>*
cUPnPControlPoint::GetAllVideos(int32 aServerID)
{
	return &cvVideosList;
}

cLinkedList<MediaObj>*
cUPnPControlPoint::GetAllPictures(int32 aServerID)
{
	return &cvPicturesList;
}


void
cUPnPControlPoint::UpdateObjectsToCache(MetaDataObj &aMetadataObj,MetaDataObj aUpdateObj)
{
	int32 count = 0;
	MetaDataObj tempObj;

	if(NULL != aMetadataObj.ChildList){
		for(count=1; count <= aMetadataObj.ChildList->GetNumberOfElements(); count++){
			os_memset(&tempObj,0,sizeof(tempObj));
			if(SUCCESS == aMetadataObj.ChildList->GetElementAtIndex(tempObj,count)){
				if((1 == tempObj.Container)){
					if(aUpdateObj.ObjectID == tempObj.ObjectID){
						aMetadataObj.ChildList->RemoveElementAtIndex(count);
						aMetadataObj.ChildList->AddAtStart(aUpdateObj);
						printf("UpdateObjectsToCache updated [%s] to Cache\n",aUpdateObj.Title);
					}else{
						UpdateObjectsToCache(tempObj,aUpdateObj);
					}
				}
			}
		}
	}
}

MetaDataObj
cUPnPControlPoint::RetrieveObjectsFromCache(int32 aObjectID,MetaDataObj &aMetadataObj)
{
	int32 count = 0;
	MetaDataObj tempObj, finalObj;

	if(aObjectID == aMetadataObj.ObjectID){
		return aMetadataObj;
	}else{
		if(1 == aMetadataObj.Container && NULL != aMetadataObj.ChildList){
			for(count=1; count <= aMetadataObj.ChildList->GetNumberOfElements(); count++){
				os_memset(&tempObj,0,sizeof(tempObj));
				if(SUCCESS == aMetadataObj.ChildList->GetElementAtIndex(tempObj,count)){
					if(aObjectID == tempObj.ObjectID){
						return tempObj;
					}else if((1 == tempObj.Container)){
						finalObj = RetrieveObjectsFromCache(aObjectID,tempObj);
						if(aObjectID == finalObj.ObjectID){
							return finalObj;
						}
					}
				}
			}
		}
	}
	/* Should not reach here.. We reached here means we didn't get this ObjectID. So return NULL..*/
	return tempObj;
}

ReturnStatus
cUPnPControlPoint::GetObjects(int32 aObjectID, cLinkedList<MetaDataObj> &aObjList, int startIndex, int limit,ContentType type)
{
	MetaDataObj tempObj;
	DeviceInfo devInfo;
	
	if(true == cvCacheServerData){
		tempObj = RetrieveObjectsFromCache(aObjectID,cvServerObjects);
		CopyObjectsToUser(tempObj,aObjList,startIndex,limit,type);
	}else{ /* Now we dont have any data cached.. So get it from the server instantly.. */
		tempObj = RetrieveObjectsFromCache(aObjectID,cvServerObjects);

		if(aObjectID == tempObj.ObjectID){
			if(true == tempObj.ObjectsAlreadyRetrieved){ /* If the Objects are already retrieved then no need to get them again.. Try to save Network Bandwidth.. :) */
				CopyObjectsToUser(tempObj,aObjList,startIndex,limit,type);
				return SUCCESS;
			}
			if(1 == tempObj.Container){
				if(tempObj.ChildList == NULL){
					tempObj.ChildList = os_new(cLinkedList<struct metaDataObj>,());
					if(NULL == tempObj.ChildList){
						LOGE("cUPnPControlPoint::GetObjects Failed to allocate the memory for child list");
						return FAILURE;
					}
				}
				devInfo = RetrieveDeviceInfo(cvActiveServerID);
				if(SUCCESS == GetDirectChilds(devInfo,tempObj,startIndex,limit)){
					if(tempObj.ObjectID == cvServerObjects.ObjectID){
						os_memcpy(&cvServerObjects,&tempObj,sizeof(cvServerObjects));
					}else{
						UpdateObjectsToCache(cvServerObjects,tempObj);
					}
					CopyObjectsToUser(tempObj,aObjList,startIndex,limit,type);
				}else{
					LOGE("cUPnPControlPoint::GetObjects Failed to get Direct Childs");
					return FAILURE;
				}
			}else{
				LOGE("cUPnPControlPoint::GetObjects Not a Container. So can not get the objects");
				return FAILURE;
			}
		}else{
			LOGE("cUPnPControlPoint::GetObjects ObjectID doesn't match");
			return FAILURE;
		}
	}
	return SUCCESS;
}

int32
cUPnPControlPoint::GetDuration(int32 aObjectID)
{
	MetaDataObj tempObj;
	//DeviceInfo devInfo;
	tempObj = RetrieveObjectsFromCache(aObjectID,cvServerObjects);
	return tempObj.Duration;
}

int32
cUPnPControlPoint::GetNumOfChilds(int32 aObjectID)
{
	MetaDataObj tempObj;
	//DeviceInfo devInfo;
	tempObj = RetrieveObjectsFromCache(aObjectID,cvServerObjects);
	if(tempObj.Container){
		if(-1 == tempObj.ChildCount){
			tempObj.ChildCount = GetChildCount(cvActiveDevInfo,tempObj.ID);
			UpdateObjectsToCache(cvServerObjects,tempObj);
		}
		return tempObj.ChildCount;
	}
	return 0;
}

ReturnStatus
cUPnPControlPoint::GetMetaData(int32 aServerID, int32 aObjectID, MetaDataObj &aMetadataObj)
{
	MetaDataObj tempObj;

	if(aServerID != cvActiveServerID){
		return FAILURE;
	}

	tempObj = RetrieveObjectsFromCache(aObjectID,cvServerObjects);

	if(aObjectID == tempObj.ObjectID){
		if(os_strlen(tempObj.Title)){
			os_strncpy(aMetadataObj.Title,tempObj.Title,sizeof(aMetadataObj.Title));
		}
		if(os_strlen(tempObj.Creator)){
			os_strncpy(aMetadataObj.Creator,tempObj.Creator,sizeof(aMetadataObj.Creator));
		}
		if(os_strlen(tempObj.Artist)){
			os_strncpy(aMetadataObj.Artist,tempObj.Artist,sizeof(aMetadataObj.Artist));
		}
		if(os_strlen(tempObj.Album)){
			os_strncpy(aMetadataObj.Album,tempObj.Album,sizeof(aMetadataObj.Album));
		}
		if(os_strlen(tempObj.Genre)){
			os_strncpy(aMetadataObj.Genre,tempObj.Genre,sizeof(aMetadataObj.Genre));
		}
		if(os_strlen(tempObj.ProtocolInfo)){
			os_strncpy(aMetadataObj.ProtocolInfo,tempObj.ProtocolInfo,sizeof(aMetadataObj.ProtocolInfo));
		}
		if(os_strlen(tempObj.URL)){
			os_strncpy(aMetadataObj.URL,tempObj.URL,sizeof(aMetadataObj.URL));
		}

		aMetadataObj.Container = tempObj.Container;
		aMetadataObj.Size = tempObj.Size;

		return SUCCESS;
	}else{
		return FAILURE;
	}
}

void
cUPnPControlPoint::HandleSignal(int32 aSignal)
{
	/* First Check the signal. If it is trivial signal then simpley Ignore. Other wise stop everthing. */
	if(OS_SIGPIPE == aSignal){
		/* No Need to Do anything here as trival signal is caught. */
		return;
	}else{
		/* Non Trivial signal is caught. So Stop all the UPnP related operations and exitfrom here itself.*/
		if(NULL != cvUPnPDiscoveryPtr){
			cvUPnPDiscoveryPtr->StopUPnPDiscovery();
			os_delete (cvUPnPDiscoveryPtr);
			cvUPnPDiscoveryPtr = NULL;
		}
	}
}

void cUPnPControlPoint::StopControlPoint()
{
	if(NULL != cvUPnPDiscoveryPtr){
		cvUPnPDiscoveryPtr->StopUPnPDiscovery();
		os_delete (cvUPnPDiscoveryPtr);
		cvUPnPDiscoveryPtr = NULL;
	}
}

void
cUPnPControlPoint::HandleSocketActivity(IN SocketObserverParams &aSockObsParms)
{
  /* No need to do anything. Becuase in this we only create TCP Client socket. So we don't get any callbacks... This is just needed to use cSocket class.. */
}


#include "upnp_discovery.h"

os_semaphore cUPnPDiscovery::cvControlPointSem;

cUPnPDiscovery::cUPnPDiscovery()
{
	cvDiscoveryActive = true;
	cvDiscoveryForServer = true; /* By default we will assume the discovery is for server.. */
	cvControPointInterfacePtr = NULL;
	
	// sMiLo
	cvLocalAddressPtr = NULL;
}

cUPnPDiscovery::~cUPnPDiscovery()
{
	while(cvInterfaceSockParmStruct.GetNumberOfElements()){
		InterfaceSockParmStruct sIFSockParmStructTemp;
		os_memset(&sIFSockParmStructTemp,'\0',sizeof(sIFSockParmStructTemp));
		cvInterfaceSockParmStruct.RemoveAtStart(sIFSockParmStructTemp);
		if(sIFSockParmStructTemp.BroadCastSocket){
			sIFSockParmStructTemp.BroadCastSocket->CloseSocket(sIFSockParmStructTemp.BroadCastSocketID);
			os_delete(sIFSockParmStructTemp.BroadCastSocket);
		}
		if(sIFSockParmStructTemp.ListenSocket){
			sIFSockParmStructTemp.ListenSocket->CloseSocket(sIFSockParmStructTemp.ListenSocketID);
			os_delete(sIFSockParmStructTemp.ListenSocket);
		}
	}
	
	// sMiLo
	if (cvLocalAddressPtr != NULL)
		os_free(cvLocalAddressPtr);
}

void cUPnPDiscovery::SetDiscoveryForServer(void)
{
	LOGD("cUPnPDiscovery::SetDiscoveryForServer Setting the discovery for Server");
	cvDiscoveryForServer = true;
}

void cUPnPDiscovery::SetDiscoveryForControlPoint(void)
{
	LOGD("cUPnPDiscovery::SetDiscoveryForServer Setting the discovery for ControlPoint");
	cvDiscoveryForServer = false;
}

void
cUPnPDiscovery::RegisterControlPointInterfacePtr(UPnPControlPointInterface *aCPIntrerfacePtr)
{
	cvControPointInterfacePtr = aCPIntrerfacePtr;
}

void cUPnPDiscovery::StartUPnPDiscovery()
{
	int32 count=0, iRetVal = 0;

	/* Initialize Mutex only in case of Control Point operations. */
	if(false == cvDiscoveryForServer){
		os_semaphore_init(&cvControlPointSem,0,1);
		os_semaphore_wait(&cvControlPointSem);
	}

	/* Look for the Active IP Interfaces and create the corresponding UDP Broadcasting socket and fill the corresponding params and push in to the linked list.*/
	for(count=1; count<=GetNumOfActiveIPAddr(); count++){
		InterfaceSockParmStruct sIFSockParmStruct;
		os_memset(&sIFSockParmStruct,'\0',sizeof(sIFSockParmStruct));

		int8* cpTempURL = GetActiveIPAddrAtIndex(count);
		if(!cpTempURL){
			LOGE("cUPnPDiscovery::StartUpnpDiscovery : Ethernet Address is invalid at index[%d]",count);
			continue;
		}
		os_memcpy(sIFSockParmStruct.InterfaceIPAddr,cpTempURL,os_strlen(cpTempURL));
		
		// sMiLo : Save local ip address
		if (cvLocalAddressPtr == NULL) {
			cvLocalAddressPtr = (int8*)os_malloc(MAXIMUM_IP_SIZE);
			os_memcpy(cvLocalAddressPtr,cpTempURL,os_strlen(cpTempURL));
		}
		
		LOGD("Starting the Discovery on address: %s",cpTempURL);
		/* Create UDP Broad Cast socket for broadcasting the NOTIFY(in case of Server) & M-SEARCH (in case of Control Point) Messages. */
		sIFSockParmStruct.BroadCastSocket = os_new(cSocket,());
		sIFSockParmStruct.BroadCastSocket->InitializeParams(sIFSockParmStruct.InterfaceIPAddr,GetSSDPInfo()->SSDPBroadcastPortNo,this);
		sIFSockParmStruct.BroadCastSocketID= sIFSockParmStruct.BroadCastSocket->CreateUDPSocket();
		if(0 > sIFSockParmStruct.BroadCastSocketID){
			/* No Need to return from Here itself. Just log the Error. That's it. In case of multiple ethernet
			   card address, if one fails then we should continue on other rather than stopping here itself. */
			LOGE("cUPnPDiscovery::StartUpnpDiscovery : CreateUDPsocket for Broadcasting Failed on the Address [%s] Port [%d]",
						  sIFSockParmStruct.InterfaceIPAddr,GetSSDPInfo()->SSDPBroadcastPortNo);
			continue;
		}
		/* Create UDP Listen socket for listining to M-SEARCH(in case of server) & NOTIFY (in case of Control Point) Messages. */
		sIFSockParmStruct.ListenSocket = os_new(cSocket,());
		sIFSockParmStruct.ListenSocket->InitializeParams(GetSSDPInfo()->SSDPBroadcastAddr,GetSSDPInfo()->SSDPBroadcastPortNo,this);
		sIFSockParmStruct.ListenSocket->SetMembershipAddr(sIFSockParmStruct.InterfaceIPAddr);
		if(cvDiscoveryForServer){
			sIFSockParmStruct.ListenSocket->SetFilteringUDPMessageString("M-SEARCH");
		}else{
			//LOG_CONSOLE("cUPnPDiscovery::StartUpnpDiscovery : calling SetFilteringUDPMessageString for NOTIFY & HTTP/1.1 200 OK");
			sIFSockParmStruct.ListenSocket->SetFilteringUDPMessageString(DIGITAL_MEDIA_SERVER_STRING);
			sIFSockParmStruct.ListenSocket->SetFilteringUDPMessageString(DIGITAL_MEDIA_RENDERER_STRING);
		}
		sIFSockParmStruct.ListenSocketID= sIFSockParmStruct.ListenSocket->CreateUDPSocket();
		if(0 > sIFSockParmStruct.ListenSocketID){
			/* No Need to return from Here itself. Just log the Error. That's it. In case of multiple ethernet
			   card address, if one fails then we should continue on other rather than stopping here itself. */
			LOGE("cUPnPDiscovery::StartUpnpDiscovery : CreateUDPsocket for Listening Failed on the Address [%s] Port [%d]",
						  sIFSockParmStruct.InterfaceIPAddr,GetSSDPInfo()->SSDPBroadcastPortNo);
			continue;
		}
		iRetVal = sIFSockParmStruct.ListenSocket->UDPUnBlockingListenCall();
		if(SUCCESS != iRetVal){
			/* No Need to return from Here itself. Just log the Error. That's it. In case of multiple ethernet
			   card address, if one fails then we should continue on other rather than stopping here itself. */
			LOGE("cUPnPDiscovery::StartUpnpDiscovery : UDPUnBlockingListenCall Failed on the Address [%s] Port [%d]",
						  GetSSDPInfo()->SSDPBroadcastAddr,GetSSDPInfo()->SSDPBroadcastPortNo);
			continue;
		}
		/* Register for Unicast and Set TTL as per UPnP Guidelines. */
		SockOptValue sValue1;
		sValue1.vpPtr = (void*)sIFSockParmStruct.InterfaceIPAddr;
		os_set_SocketOptions(sIFSockParmStruct.BroadCastSocketID,MULTICAST_INTERFACE,&sValue1);
		sValue1.vpPtr = NULL;
		sValue1.iValue = 4;
		os_set_SocketOptions(sIFSockParmStruct.BroadCastSocketID,TTL,&sValue1);

		cvInterfaceSockParmStruct.Add(sIFSockParmStruct);
	}

	/* By Here we got the list of UDP sockets for all active internet interfaces. Start broadcasting the UDP messages on all Interfaces. */
	if(cvDiscoveryForServer){ /* for Server we need to send the NOTIFY packets periodically. So do that.... */
		while(cvDiscoveryActive){
			for(count=1; count <= cvInterfaceSockParmStruct.GetNumberOfElements(); count++){
				InterfaceSockParmStruct sIFSockParmStructTemp;
				os_memset(&sIFSockParmStructTemp,'\0',sizeof(sIFSockParmStructTemp));
				cvInterfaceSockParmStruct.GetElementAtIndex(sIFSockParmStructTemp,count);
				LOGD("cUPnPDiscovery::StartUpnpDiscovery sending NOTIFY packets on IP[%s]",sIFSockParmStructTemp.InterfaceIPAddr);
				SendNotifyPackets(&sIFSockParmStructTemp);
			}
			os_sleep(GetSSDPInfo()->SSDPDiscoveryMsgLifetime*1000);
		}
	}else{ /* In case of Control point it is sufficient sending M-SEARCH packets once CP is started and after that it can continuously listen for NOTIFY messages. It don't need to send the periodic events.. So do it only once..*/
		for(count=1; count <= cvInterfaceSockParmStruct.GetNumberOfElements(); count++){
			InterfaceSockParmStruct sIFSockParmStructTemp;
			os_memset(&sIFSockParmStructTemp,'\0',sizeof(sIFSockParmStructTemp));
			cvInterfaceSockParmStruct.GetElementAtIndex(sIFSockParmStructTemp,count);
			LOGD("cUPnPDiscovery::StartUpnpDiscovery sending M-SEARCH packets on IP[%s]",sIFSockParmStructTemp.InterfaceIPAddr);
			SendMSearchPackets(&sIFSockParmStructTemp);
			sIFSockParmStructTemp.BroadCastSocket->SetFilteringUDPMessageString(DIGITAL_MEDIA_SERVER_STRING);
			sIFSockParmStructTemp.BroadCastSocket->SetFilteringUDPMessageString(DIGITAL_MEDIA_RENDERER_STRING);
			iRetVal = sIFSockParmStructTemp.BroadCastSocket->UDPUnBlockingListenCall();
			if(SUCCESS != iRetVal){
				// No Need to return from Here itself. Just log the Error. That's it. In case of multiple ethernet
				//  card address, if one fails then we should continue on other rather than stopping here itself.
				LOGE("cUPnPDiscovery::StartUpnpDiscovery : UDPUnBlockingListenCall Failed for Boradcast socket for listening the return messages");
			}
		}

		os_sleep(UPnP_DISCOVERY_MX_TIME_INTEVAL_IN_M_SEARCH*1000);

		for(count=1; count <= cvInterfaceSockParmStruct.GetNumberOfElements(); count++){
			InterfaceSockParmStruct sIFSockParmStructTemp;
			os_memset(&sIFSockParmStructTemp,'\0',sizeof(sIFSockParmStructTemp));
			cvInterfaceSockParmStruct.GetElementAtIndex(sIFSockParmStructTemp,count);
			LOGD("cUPnPDiscovery::StartUpnpDiscovery Closing the UDP Broadcast socket after MX interval mentioned in the broadcast packet");
			sIFSockParmStructTemp.BroadCastSocket->CloseConnection();
		}
		os_semaphore_wait(&cvControlPointSem); /* Here the control gets struct. And when os_mutex_unlock from StopUPnPDiscovery is called then mutex unlock happens.*/
		os_semaphore_close(&cvControlPointSem); /* Immediately after comming out of it distroy the mutex as there is no use of it any more.. */
	}
}

void cUPnPDiscovery::StopUPnPDiscovery()
{
	if(true == cvDiscoveryForServer){ /* Send Bye-Bye Packets only incase of Server.. In case of Control Point no need to send anything..*/
		/* First send Bye Bye Packets on All interfaces before Going down. */
		SendByeByePackets();
	}
	cvDiscoveryActive = false;
	/* Go thorough the list of all interfaces and close all the Broadcasting and listen socket IDS. */
	for(int32 count=1; count <= cvInterfaceSockParmStruct.GetNumberOfElements(); count++){
		InterfaceSockParmStruct sIFSockParmStructTemp;
		os_memset(&sIFSockParmStructTemp,'\0',sizeof(sIFSockParmStructTemp));
		cvInterfaceSockParmStruct.GetElementAtIndex(sIFSockParmStructTemp,count);
		LOGD("cUPnPDiscovery::StopUPnPDiscovery Closing the sockets on IP[%s] BroadcastSockID[%d] ListenSocketID[%d]",
					  sIFSockParmStructTemp.InterfaceIPAddr,
					  sIFSockParmStructTemp.BroadCastSocketID,
					  sIFSockParmStructTemp.ListenSocketID);
		sIFSockParmStructTemp.BroadCastSocket->StopSocketServer();
		cSocket::CloseSocket(sIFSockParmStructTemp.BroadCastSocketID);
		cSocket::CloseSocket(sIFSockParmStructTemp.ListenSocketID);
	}
	if(false == cvDiscoveryForServer){ /* In case of Control Point we need to unlock the Mutex.. */
		os_semaphore_post(&cvControlPointSem);
	}

}

void cUPnPDiscovery::SendNotifyPackets(InterfaceSockParmStruct* aIFSockParmStruct)
{
	int8 ca512Buf[512] = {'\0'};
	int32 iNoEmbedDevices = GetEnbedDeviceList()->GetNumberOfElements();
	int32 iNoServices = GetServiceIds()->GetNumberOfElements();
	int32 iCount=0;

	os_memset(ca512Buf,'\0',sizeof(ca512Buf));
	CreateNotifyPacket(NOTIFY_GUID,GetRootDeviceInfo(),aIFSockParmStruct->InterfaceIPAddr,ca512Buf,sizeof(ca512Buf));
	LogSSDPMessages(ca512Buf);
	aIFSockParmStruct->BroadCastSocket->SendTo(aIFSockParmStruct->BroadCastSocketID,ca512Buf,os_strlen(ca512Buf),GetSSDPInfo()->SSDPBroadcastAddr,
				  GetSSDPInfo()->SSDPBroadcastPortNo);

	os_memset(ca512Buf,'\0',sizeof(ca512Buf));
	CreateNotifyPacket(NOTIFY_ROOT,GetRootDeviceInfo(),aIFSockParmStruct->InterfaceIPAddr,ca512Buf,sizeof(ca512Buf));
	LogSSDPMessages(ca512Buf);
	aIFSockParmStruct->BroadCastSocket->SendTo(aIFSockParmStruct->BroadCastSocketID,ca512Buf,os_strlen(ca512Buf),GetSSDPInfo()->SSDPBroadcastAddr,
				  GetSSDPInfo()->SSDPBroadcastPortNo);

	os_memset(ca512Buf,'\0',sizeof(ca512Buf));
	CreateNotifyPacket(NOTIFY_DEVICE,GetRootDeviceInfo(),aIFSockParmStruct->InterfaceIPAddr,ca512Buf,sizeof(ca512Buf));
	LogSSDPMessages(ca512Buf);
	aIFSockParmStruct->BroadCastSocket->SendTo(aIFSockParmStruct->BroadCastSocketID,ca512Buf,os_strlen(ca512Buf),GetSSDPInfo()->SSDPBroadcastAddr,
				  GetSSDPInfo()->SSDPBroadcastPortNo);


	for(iCount=1; iCount<=iNoEmbedDevices; iCount++){
		SSDPDeviceInfoStruct sDeviceStructTemp;
		if(FAILURE != GetEnbedDeviceList()->GetElementAtIndex(sDeviceStructTemp,iCount)){
			os_memset(ca512Buf,'\0',sizeof(ca512Buf));
			CreateNotifyPacket(NOTIFY_DEVICE,&sDeviceStructTemp,aIFSockParmStruct->InterfaceIPAddr,ca512Buf,sizeof(ca512Buf));
			LogSSDPMessages(ca512Buf);
			aIFSockParmStruct->BroadCastSocket->SendTo(aIFSockParmStruct->BroadCastSocketID,ca512Buf,os_strlen(ca512Buf),GetSSDPInfo()->SSDPBroadcastAddr,
						  GetSSDPInfo()->SSDPBroadcastPortNo);
		}
	}


	for(iCount=1; iCount<=iNoServices; iCount++){
		SSDPServiceID serviceID;
		if(FAILURE != GetServiceIds()->GetElementAtIndex(serviceID,iCount)){
			os_memset(ca512Buf,'\0',sizeof(ca512Buf));
			CreateNotifyPacket(NOTIFY_SERVICE,&serviceID,aIFSockParmStruct->InterfaceIPAddr,ca512Buf,sizeof(ca512Buf));
			LogSSDPMessages(ca512Buf);
			aIFSockParmStruct->BroadCastSocket->SendTo(aIFSockParmStruct->BroadCastSocketID,ca512Buf,os_strlen(ca512Buf),GetSSDPInfo()->SSDPBroadcastAddr,
						  GetSSDPInfo()->SSDPBroadcastPortNo);
		}
	}
}

void cUPnPDiscovery::CreateNotifyPacket(IN const cvUDPPacketType aType,IN const void* vpInStruct, IN const int8* aDDIP, OUT int8* aOutBuf,
					IN const int32 aOutBufLen)
{
	if(NOTIFY_ROOT == aType || NOTIFY_GUID == aType){
		SSDPDeviceInfoStruct *sDeviceStruct = (SSDPDeviceInfoStruct*)vpInStruct;
		os_snprintf(aOutBuf,aOutBufLen,
			      "NOTIFY * HTTP/1.1\r\n"
			      "HOST: %s:%d\r\n"
			      "CACHE-CONTROL: max-age=%u\r\n"
			      "LOCATION: http://%s:%d%s\r\n"
			      "NT: %s%s\r\n"
			      "NTS: ssdp:alive\r\n"
			      "SERVER: %s/%d.%d UPnP/1.0 %s/%d.%d\r\n"
			      "USN: uuid:%s%s%s\r\n"
			      "\r\n",
			      GetSSDPInfo()->SSDPBroadcastAddr,
			      GetSSDPInfo()->SSDPBroadcastPortNo,
			      GetSSDPInfo()->SSDPDiscoveryMsgLifetime,
			      aDDIP,
			      GetSSDPInfo()->SSDPDDPort,
			      DEVICE_DESCRIPTION_STRING,
			      (NOTIFY_GUID == aType)?"uuid:":"",
			      (NOTIFY_ROOT == aType)?"upnp:rootdevice":sDeviceStruct->DeviceUUID,
			      GetSSDPInfo()->SSDPOSName,
			      GetSSDPInfo()->SSDPOSMajorVersionNo,
			      GetSSDPInfo()->SSDPOSMinorVersionNo,
			      GetSSDPInfo()->SSDPProductName,
			      GetSSDPInfo()->SSDPProductMajorVersionNo,
			      GetSSDPInfo()->SSDPProductMinorVersionNo,
			      sDeviceStruct->DeviceUUID,
			      (NOTIFY_ROOT == aType)?"::":"",
			      (NOTIFY_ROOT == aType)?"upnp:rootdevice":"");
	}else if(NOTIFY_DEVICE == aType){
		SSDPDeviceInfoStruct *sDeviceStruct = (SSDPDeviceInfoStruct*)vpInStruct;
		os_snprintf(aOutBuf,aOutBufLen,
			      "NOTIFY * HTTP/1.1\r\n"
			      "HOST: %s:%d\r\n"
			      "CACHE-CONTROL: max-age=%u\r\n"
			      "LOCATION: http://%s:%d%s\r\n"
			      "NT: urn:schemas-upnp-org:device:%s\r\n"
			      "NTS: ssdp:alive\r\n"
			      "SERVER: %s/%d.%d UPnP/1.0 %s/%d.%d\r\n"
			      "USN: uuid:%s::urn:schemas-upnp-org:device:%s\r\n"
			      "\r\n",
			      GetSSDPInfo()->SSDPBroadcastAddr,
			      GetSSDPInfo()->SSDPBroadcastPortNo,
			      GetSSDPInfo()->SSDPDiscoveryMsgLifetime,
			      aDDIP,
			      GetSSDPInfo()->SSDPDDPort,
			      DEVICE_DESCRIPTION_STRING,
			      sDeviceStruct->DeviceName,
			      GetSSDPInfo()->SSDPOSName,
			      GetSSDPInfo()->SSDPOSMajorVersionNo,
			      GetSSDPInfo()->SSDPOSMinorVersionNo,
			      GetSSDPInfo()->SSDPProductName,
			      GetSSDPInfo()->SSDPProductMajorVersionNo,
			      GetSSDPInfo()->SSDPProductMinorVersionNo,
			      sDeviceStruct->DeviceUUID,
			      sDeviceStruct->DeviceName);
	}else if(NOTIFY_SERVICE == aType){
		SSDPServiceID serviceID = *(SSDPServiceID*)vpInStruct;
		int8 temp[64] = {'\0'};
		/* Identify the service related strings based on the incomming service ID. */
		if(CONTENT_DIRECTORY_SERVICE_ID == serviceID){
			os_strncpy(temp,CONTENT_DIRECTORY_SERVICE_TYPE,sizeof(temp));
		}else if(CONNECTION_MANAGER_SERVICE_ID == serviceID){
			os_strncpy(temp,CONNECTION_MANAGER_SERVICE_TYPE,sizeof(temp));
		}

		os_snprintf(aOutBuf,aOutBufLen,
			      "NOTIFY * HTTP/1.1\r\n"
			      "HOST: %s:%d\r\n"
			      "CACHE-CONTROL: max-age=%u\r\n"
			      "LOCATION: http://%s:%d%s\r\n"
			      "NT: %s\r\n"
			      "NTS: ssdp:alive\r\n"
			      "SERVER: %s/%d.%d UPnP/1.0 %s/%d.%d\r\n"
			      "USN: uuid:%s::%s\r\n"
			      "\r\n",
			      GetSSDPInfo()->SSDPBroadcastAddr,
			      GetSSDPInfo()->SSDPBroadcastPortNo,
			      GetSSDPInfo()->SSDPDiscoveryMsgLifetime,
			      aDDIP,
			      GetSSDPInfo()->SSDPDDPort,
			      DEVICE_DESCRIPTION_STRING,
			      temp,
			      GetSSDPInfo()->SSDPOSName,
			      GetSSDPInfo()->SSDPOSMajorVersionNo,
			      GetSSDPInfo()->SSDPOSMinorVersionNo,
			      GetSSDPInfo()->SSDPProductName,
			      GetSSDPInfo()->SSDPProductMajorVersionNo,
			      GetSSDPInfo()->SSDPProductMinorVersionNo,
			      GetRootDeviceInfo()->DeviceUUID,
			      temp);
	}else if(MSEARCH_ROOT == aType){
		SSDPDeviceInfoStruct *sDeviceStruct = (SSDPDeviceInfoStruct*)vpInStruct;
		os_snprintf(aOutBuf,aOutBufLen,
			      "HTTP/1.1 200 OK\r\n"
			      "CACHE-CONTROL: max-age=%u\r\n"
			      "EXT:\r\n"
			      "LOCATION: http://%s:%d%s\r\n"
			      "SERVER: %s/%d.%d UPnP/1.0 %s/%d.%d\r\n"
			      "ST: upnp:rootdevice\r\n"
			      "USN: uuid:%s::upnp:rootdevice\r\n"
			      "\r\n",
			      GetSSDPInfo()->SSDPDiscoveryMsgLifetime,
			      aDDIP,
			      GetSSDPInfo()->SSDPDDPort,
			      DEVICE_DESCRIPTION_STRING,
			      GetSSDPInfo()->SSDPOSName,
			      GetSSDPInfo()->SSDPOSMajorVersionNo,
			      GetSSDPInfo()->SSDPOSMinorVersionNo,
			      GetSSDPInfo()->SSDPProductName,
			      GetSSDPInfo()->SSDPProductMajorVersionNo,
			      GetSSDPInfo()->SSDPProductMinorVersionNo,
			      sDeviceStruct->DeviceUUID);
	}else if(MSEARCH_DEVICE == aType){
		SSDPDeviceInfoStruct *sDeviceStruct = (SSDPDeviceInfoStruct*)vpInStruct;
		os_snprintf(aOutBuf,aOutBufLen,
			      "HTTP/1.1 200 OK\r\n"
			      "CACHE-CONTROL: max-age=%u\r\n"
			      "EXT:\r\n"
			      "LOCATION: http://%s:%d%s\r\n"
			      "SERVER: %s/%d.%d UPnP/1.0 %s/%d.%d\r\n"
			      "ST:  urn:schemas-upnp-org:device:%s\r\n"
			      "USN: uuid:%s::urn:schemas-upnp-org:device:%s\r\n"
			      "\r\n",
			      GetSSDPInfo()->SSDPDiscoveryMsgLifetime,
			      aDDIP,
			      GetSSDPInfo()->SSDPDDPort,
			      DEVICE_DESCRIPTION_STRING,
			      GetSSDPInfo()->SSDPOSName,
			      GetSSDPInfo()->SSDPOSMajorVersionNo,
			      GetSSDPInfo()->SSDPOSMinorVersionNo,
			      GetSSDPInfo()->SSDPProductName,
			      GetSSDPInfo()->SSDPProductMajorVersionNo,
			      GetSSDPInfo()->SSDPProductMinorVersionNo,
			      sDeviceStruct->DeviceName,
			      sDeviceStruct->DeviceUUID,
			      sDeviceStruct->DeviceName);
	}else if(MSEARCH_SERVICE == aType){
		SSDPServiceID serviceID = *(SSDPServiceID*)vpInStruct;
		int8 temp[64] = {'\0'};
		/* Identify the service related strings based on the incomming service ID. */
		if(CONTENT_DIRECTORY_SERVICE_ID == serviceID){
			os_strncpy(temp,CONTENT_DIRECTORY_SERVICE_TYPE,sizeof(temp));
		}else if(CONNECTION_MANAGER_SERVICE_ID == serviceID){
			os_strncpy(temp,CONNECTION_MANAGER_SERVICE_TYPE,sizeof(temp));
		}
		os_snprintf(aOutBuf,aOutBufLen,
			      "HTTP/1.1 200 OK\r\n"
			      "CACHE-CONTROL: max-age=%u\r\n"
			      "EXT:\r\n"
			      "LOCATION: http://%s:%d%s\r\n"
			      "SERVER: %s/%d.%d UPnP/1.0 %s/%d.%d\r\n"
			      "NT: %s\r\n"
			      "USN: uuid:%s::%s\r\n"
			      "\r\n",
			      GetSSDPInfo()->SSDPDiscoveryMsgLifetime,
			      aDDIP,
			      GetSSDPInfo()->SSDPDDPort,
			      DEVICE_DESCRIPTION_STRING,
			      GetSSDPInfo()->SSDPOSName,
			      GetSSDPInfo()->SSDPOSMajorVersionNo,
			      GetSSDPInfo()->SSDPOSMinorVersionNo,
			      GetSSDPInfo()->SSDPProductName,
			      GetSSDPInfo()->SSDPProductMajorVersionNo,
			      GetSSDPInfo()->SSDPProductMinorVersionNo,
			      temp,
			      GetRootDeviceInfo()->DeviceUUID,
			      temp);
	}
}

void cUPnPDiscovery::SendMSearchPackets(InterfaceSockParmStruct* aIFSockParmStruct)
{
	int8 ca512Buf[512] = {'\0'};

#if 1 /* Just for a trial to get all the advertisements.. In future we can disable this as we are only intrested in MediaServers and MediaRenderes.. */
	/* Send M-SEARCH for Media Servers. */
	os_memset(ca512Buf,'\0',sizeof(ca512Buf));
	CreateMSearchPacket(ALL,ca512Buf,sizeof(ca512Buf));
	LogSSDPMessages(ca512Buf);
	aIFSockParmStruct->BroadCastSocket->SendTo(aIFSockParmStruct->BroadCastSocketID,ca512Buf,os_strlen(ca512Buf),GetSSDPInfo()->SSDPBroadcastAddr,
				  GetSSDPInfo()->SSDPBroadcastPortNo);
#endif

	/* Send M-SEARCH for Media Servers. */
	os_memset(ca512Buf,'\0',sizeof(ca512Buf));
	CreateMSearchPacket(MEDIA_SERVER,ca512Buf,sizeof(ca512Buf));
	LogSSDPMessages(ca512Buf);
	aIFSockParmStruct->BroadCastSocket->SendTo(aIFSockParmStruct->BroadCastSocketID,ca512Buf,os_strlen(ca512Buf),GetSSDPInfo()->SSDPBroadcastAddr,
				  GetSSDPInfo()->SSDPBroadcastPortNo);

	/* Send M-SEARCH for Media Renderers. */
	os_memset(ca512Buf,'\0',sizeof(ca512Buf));
	CreateMSearchPacket(MEDIA_RENDERER,ca512Buf,sizeof(ca512Buf));
	LogSSDPMessages(ca512Buf);
	aIFSockParmStruct->BroadCastSocket->SendTo(aIFSockParmStruct->BroadCastSocketID,ca512Buf,os_strlen(ca512Buf),GetSSDPInfo()->SSDPBroadcastAddr,
				  GetSSDPInfo()->SSDPBroadcastPortNo);

}

void cUPnPDiscovery::CreateMSearchPacket(IN const MsearchTarget aType, INOUT int8* aOutBuf, IN const int32 aOutBufLen)
{
	os_snprintf(aOutBuf,aOutBufLen,
		    "M-SEARCH * HTTP/1.1\r\n"
		    "HOST: 239.255.255.250:1900\r\n"
		    "MAN: \"ssdp:discover\"\r\n"
		    "MX: %d\r\n"
		    "ST: %s\r\n"
		    "\r\n",
		    UPnP_DISCOVERY_MX_TIME_INTEVAL_IN_M_SEARCH,
		    (ALL == aType) ? "ssdp:all" :
		    (ROOT_DEVICE == aType) ? "upnp:rootdevice" :
		    (MEDIA_SERVER == aType) ? DIGITAL_MEDIA_SERVER_STRING :
		    (MEDIA_RENDERER == aType) ? DIGITAL_MEDIA_RENDERER_STRING : ""); /* TODO: Here please be reminded that as of now we are using M-SEARCH for only all and root devices. In future we can add more based on the requirement */
}

void cUPnPDiscovery::SendByeByePackets()
{
	int8 ca512Buf[512] = {'\0'};
	int32 iNoEmbedDevices = GetEnbedDeviceList()->GetNumberOfElements();
	int32 iNoServices = GetServiceIds()->GetNumberOfElements();
	int32 iCount=0;

	for(int32 count=1; count <= cvInterfaceSockParmStruct.GetNumberOfElements(); count++){

		InterfaceSockParmStruct sIFSockParmStructTemp;
		os_memset(&sIFSockParmStructTemp,'\0',sizeof(sIFSockParmStructTemp));
		if(SUCCESS == cvInterfaceSockParmStruct.GetElementAtIndex(sIFSockParmStructTemp,count)){
			LOGD("cUPnPDiscovery::SendByeByePackets Sending ByeBye Packets on IP[%s]",sIFSockParmStructTemp.InterfaceIPAddr);


			os_memset(ca512Buf,'\0',sizeof(ca512Buf));
			CreateByeByePacket(NOTIFY_GUID,NULL,ca512Buf,sizeof(ca512Buf));
			LogSSDPMessages(ca512Buf);
			sIFSockParmStructTemp.BroadCastSocket->SendTo(sIFSockParmStructTemp.BroadCastSocketID,ca512Buf,os_strlen(ca512Buf),GetSSDPInfo()->SSDPBroadcastAddr,
						  GetSSDPInfo()->SSDPBroadcastPortNo);


			os_memset(ca512Buf,'\0',sizeof(ca512Buf));
			CreateByeByePacket(NOTIFY_ROOT,NULL,ca512Buf,sizeof(ca512Buf));
			LogSSDPMessages(ca512Buf);
			sIFSockParmStructTemp.BroadCastSocket->SendTo(sIFSockParmStructTemp.BroadCastSocketID,ca512Buf,os_strlen(ca512Buf),GetSSDPInfo()->SSDPBroadcastAddr,
						  GetSSDPInfo()->SSDPBroadcastPortNo);

			os_memset(ca512Buf,'\0',sizeof(ca512Buf));
			CreateByeByePacket(NOTIFY_DEVICE,GetRootDeviceInfo()->DeviceName,ca512Buf,sizeof(ca512Buf));
			LogSSDPMessages(ca512Buf);
			sIFSockParmStructTemp.BroadCastSocket->SendTo(sIFSockParmStructTemp.BroadCastSocketID,ca512Buf,os_strlen(ca512Buf),GetSSDPInfo()->SSDPBroadcastAddr,
						  GetSSDPInfo()->SSDPBroadcastPortNo);


			for(iCount=1; iCount<=iNoEmbedDevices; iCount++){
				SSDPDeviceInfoStruct sDeviceStructTemp;
				if(FAILURE != GetEnbedDeviceList()->GetElementAtIndex(sDeviceStructTemp,iCount)){
					os_memset(ca512Buf,'\0',sizeof(ca512Buf));
					CreateByeByePacket(NOTIFY_DEVICE,sDeviceStructTemp.DeviceName,ca512Buf,sizeof(ca512Buf));
					LogSSDPMessages(ca512Buf);
					sIFSockParmStructTemp.BroadCastSocket->SendTo(sIFSockParmStructTemp.BroadCastSocketID,ca512Buf,os_strlen(ca512Buf),GetSSDPInfo()->SSDPBroadcastAddr,
								  GetSSDPInfo()->SSDPBroadcastPortNo);
				}
			}


			for(iCount=1; iCount<=iNoServices; iCount++){
				SSDPServiceID serviceID;
				if(FAILURE != GetServiceIds()->GetElementAtIndex(serviceID,iCount)){
					os_memset(ca512Buf,'\0',sizeof(ca512Buf));
					if(CONTENT_DIRECTORY_SERVICE_ID == serviceID){
						CreateByeByePacket(NOTIFY_SERVICE,CONTENT_DIRECTORY_SERVICE_TYPE,ca512Buf,sizeof(ca512Buf));
					}else if(CONNECTION_MANAGER_SERVICE_ID == serviceID){
						CreateByeByePacket(NOTIFY_SERVICE,CONNECTION_MANAGER_SERVICE_TYPE,ca512Buf,sizeof(ca512Buf));
					}
					LogSSDPMessages(ca512Buf);
					sIFSockParmStructTemp.BroadCastSocket->SendTo(sIFSockParmStructTemp.BroadCastSocketID,ca512Buf,os_strlen(ca512Buf),GetSSDPInfo()->SSDPBroadcastAddr,
								  GetSSDPInfo()->SSDPBroadcastPortNo);
				}
			}
		}
	}
}

void cUPnPDiscovery::CreateByeByePacket(IN const cvUDPPacketType aType, IN const int8* aTargetName, OUT int8* aOutBuf,
					IN const int32 aOutBufLen)
{
	int8 targetString[128] = {'\0'};
	if(NOTIFY_ROOT == aType || NOTIFY_GUID == aType){
		os_strncpy(targetString,"upnp:rootdevice",sizeof(targetString));
	}else if(NOTIFY_DEVICE == aType){
		os_snprintf(targetString,sizeof(targetString),"urn:schemas-upnp-org:device:%s",aTargetName);
	}else if(NOTIFY_SERVICE == aType){
		os_snprintf(targetString,sizeof(targetString),"%s",aTargetName);
	}
	os_snprintf(aOutBuf,aOutBufLen,
		    "NOTIFY * HTTP/1.1\r\n"
		    "HOST: %s:%d\r\n"
		    "NT: %s%s\r\n"
		    "NTS: ssdp:byebye\r\n"
		    "USN: uuid:%s%s%s\r\n"
		    "\r\n",
		    GetSSDPInfo()->SSDPBroadcastAddr,
		    GetSSDPInfo()->SSDPBroadcastPortNo,
		    (NOTIFY_GUID == aType)?"uuid:":"",
		    (NOTIFY_GUID != aType)?targetString:GetRootDeviceInfo()->DeviceUUID,
		    GetRootDeviceInfo()->DeviceUUID,
		    (NOTIFY_GUID != aType)?"::":"",
		    (NOTIFY_GUID != aType)?targetString:"");
}

void cUPnPDiscovery::LogSSDPMessages(int8* cpInBuf)
{
#ifdef LOG_SSDP_MESSAGES
      LOGD(cpInBuf);
#endif
}

void cUPnPDiscovery::SendMSearchResponse(IN InterfaceSockParmStruct* aIFSockParmStruct,IN UDPSocketInfo &aUDPSockParam,
							      cvUDPPacketType aMsearchTarget,const int8* aItemName)
{
	int8 ca512Buf[512] = {'\0'};
	int32 iNoEmbedDevices = GetEnbedDeviceList()->GetNumberOfElements();
	int32 iNoServices = GetServiceIds()->GetNumberOfElements();
	int32 iCount=0;
	SOCKET sockID = aUDPSockParam.SockID;
#if 0
	UPnP_DISCOVERY_LOG_TRACE("cUPnPDiscovery::SendMSearchResponseForAll Notifying about NOTIFY_GUID");
	os_memset(ca512Buf,'\0',sizeof(ca512Buf));
	CreateNotifyPacket(NOTIFY_GUID,GetRootDeviceInfo(),aIFSockParmStruct->InterfaceIPAddr,ca512Buf,sizeof(ca512Buf));
	LogSSDPMessages(ca512Buf);
	aIFSockParmStruct->ListenSocket->SendTo(sockID,ca512Buf,os_strlen(ca512Buf),aUDPSockParam.PeerIPPtr,
						aUDPSockParam.PeerPort);
#endif
	if(MSEARCH_ALL == aMsearchTarget || MSEARCH_ROOT == aMsearchTarget){
		os_memset(ca512Buf,'\0',sizeof(ca512Buf));
		CreateNotifyPacket(MSEARCH_ROOT,GetRootDeviceInfo(),aIFSockParmStruct->InterfaceIPAddr,ca512Buf,sizeof(ca512Buf));
		LogSSDPMessages(ca512Buf);
		aIFSockParmStruct->ListenSocket->SendTo(sockID,ca512Buf,os_strlen(ca512Buf),aUDPSockParam.PeerIPPtr,
							aUDPSockParam.PeerPort);
	}
	if(MSEARCH_ALL == aMsearchTarget || (MSEARCH_DEVICE == aMsearchTarget &&
	   (0 == os_strcmp(aItemName,GetRootDeviceInfo()->DeviceName)))){
		os_memset(ca512Buf,'\0',sizeof(ca512Buf));
		CreateNotifyPacket(MSEARCH_DEVICE,GetRootDeviceInfo(),aIFSockParmStruct->InterfaceIPAddr,ca512Buf,sizeof(ca512Buf));
		LogSSDPMessages(ca512Buf);
		aIFSockParmStruct->ListenSocket->SendTo(sockID,ca512Buf,os_strlen(ca512Buf),aUDPSockParam.PeerIPPtr,
							aUDPSockParam.PeerPort);
	}

	for(iCount=1; iCount<=iNoEmbedDevices; iCount++){
		SSDPDeviceInfoStruct sDeviceStructTemp;
		if(FAILURE != GetEnbedDeviceList()->GetElementAtIndex(sDeviceStructTemp,iCount)){
			if(MSEARCH_ALL == aMsearchTarget || (MSEARCH_DEVICE == aMsearchTarget &&
			   (0 == os_strcmp(aItemName,sDeviceStructTemp.DeviceName)))){
				os_memset(ca512Buf,'\0',sizeof(ca512Buf));
				CreateNotifyPacket(MSEARCH_DEVICE,&sDeviceStructTemp,aIFSockParmStruct->InterfaceIPAddr,ca512Buf,sizeof(ca512Buf));
				LogSSDPMessages(ca512Buf);
				aIFSockParmStruct->ListenSocket->SendTo(sockID,ca512Buf,os_strlen(ca512Buf),aUDPSockParam.PeerIPPtr,
									    aUDPSockParam.PeerPort);
			}
		}
	}

	for(iCount=1; iCount<=iNoServices; iCount++){
		SSDPServiceID serviceID;
		if(FAILURE != GetServiceIds()->GetElementAtIndex(serviceID,iCount)){
			int8 temp[64] = {'\0'};
			/* Identify the service related strings based on the incomming service ID. */
			if(CONTENT_DIRECTORY_SERVICE_ID == serviceID){
				os_strncpy(temp,CONTENT_DIRECTORY_SERVICE_TYPE,sizeof(temp));
			}else if(CONNECTION_MANAGER_SERVICE_ID == serviceID){
				/* Copy the string accordingly. Currently as we didn't added that service we left empty here. */
			}
			if(MSEARCH_ALL == aMsearchTarget || (MSEARCH_SERVICE == aMsearchTarget &&
			   (0 == os_strcmp(aItemName,temp)))){
				os_memset(ca512Buf,'\0',sizeof(ca512Buf));
				CreateNotifyPacket(MSEARCH_SERVICE,&serviceID,aIFSockParmStruct->InterfaceIPAddr,ca512Buf,sizeof(ca512Buf));
				LogSSDPMessages(ca512Buf);
				aIFSockParmStruct->ListenSocket->SendTo(sockID,ca512Buf,os_strlen(ca512Buf),aUDPSockParam.PeerIPPtr,
									    aUDPSockParam.PeerPort);
			}
		}
	}
}

ReturnStatus cUPnPDiscovery::CreateAndSendMSearchResponse(IN UDPSocketInfo &aUDPSockParam,uint32 aMXVal,
							      cvUDPPacketType aMsearchTarget,const int8* aItemName)
{
	int32 count=0;
	for(count=1; count <= cvInterfaceSockParmStruct.GetNumberOfElements(); count++){
		InterfaceSockParmStruct sIFSockParmStructTemp;
		os_memset(&sIFSockParmStructTemp,'\0',sizeof(sIFSockParmStructTemp));
		cvInterfaceSockParmStruct.GetElementAtIndex(sIFSockParmStructTemp,count);
		SendMSearchResponse(&sIFSockParmStructTemp,aUDPSockParam,aMsearchTarget,aItemName);
	}
	return SUCCESS;
}
ReturnStatus cUPnPDiscovery::VerifyUDPMessageForMSearch(IN UDPSocketInfo &aUDPSockParam)
{
	int8 *udpMessage = (int8*)aUDPSockParam.RecvBufPtr,*tempPtr = NULL,*tempPtr1 = NULL;
	int8 mxValString[20] = {'\0'}, stString[128] = {'\0'},itemType[64] = {'\0'};
	uint32 mxVal = 0;
	cvUDPPacketType msearchTarget = MSEARCH_ALL;
	ReturnStatus status = SUCCESS;
	LOGD("cUPnPDiscovery::VerifyUDPMessageForMSearch IN for message from: 5s",aUDPSockParam.PeerIPPtr);
	if(NULL == os_strcasestr(udpMessage,"M-SEARCH * HTTP/1.1")){
		status = FAILURE;
		LOGE("cUPnPDiscovery::VerifyUDPMessageForMSearch Failed as it was unable to find M-SEARCH string");
		goto VerifyUDPMessageForMSearch_Exit;
	}

	tempPtr = os_strcasestr(udpMessage,"\r\nHOST:");
	if(NULL == tempPtr){
		status = FAILURE;
		LOGE("cUPnPDiscovery::VerifyUDPMessageForMSearch Failed as it was unable to find HOST string");
		goto VerifyUDPMessageForMSearch_Exit;
	}

	tempPtr += 7;
	while(' ' == *tempPtr || ':' == *tempPtr){
		tempPtr++;
	}

	if(0 != os_strncasecmp(tempPtr,"239.255.255.250",15)){
		status = FAILURE;
		LOGE("cUPnPDiscovery::VerifyUDPMessageForMSearch Failed as it was unable to find correct HOST string");
		goto VerifyUDPMessageForMSearch_Exit;
	}

	tempPtr = os_strcasestr(udpMessage,"\r\nMAN:");
	if(NULL == tempPtr){
		status = FAILURE;
		LOGE("cUPnPDiscovery::VerifyUDPMessageForMSearch Failed as it was unable to find MAN string");
		goto VerifyUDPMessageForMSearch_Exit;
	}

	tempPtr += 6;
	while(' ' == *tempPtr || ':' == *tempPtr){
		tempPtr++;
	}

	if(0 != os_strncasecmp(tempPtr,"\"ssdp:discover\"",15)){
		status = FAILURE;
		LOGE("cUPnPDiscovery::VerifyUDPMessageForMSearch Failed as it was unable to find correct HOST string");
		goto VerifyUDPMessageForMSearch_Exit;
	}

	/* Search for the MX string and get it's value. */
	tempPtr = os_strcasestr(udpMessage,"\r\nMX:");
	if(NULL == tempPtr){
		LOGE("cUPnPDiscovery::VerifyUDPMessageForMSearch Failed as it was unable to find MX string");
		status = FAILURE;
		goto VerifyUDPMessageForMSearch_Exit;
	}else{
		tempPtr += 5;
		while(' ' == *tempPtr){
			tempPtr++;
		}
		tempPtr1 = mxValString;
		while(' ' != *tempPtr && '\0' != *tempPtr && '\r' != *tempPtr && '\n' != *tempPtr){
			*tempPtr1++ = *tempPtr++;
		}
		if(os_strlen(mxValString)){
			mxVal = os_atoi(mxValString);
		}
	}

	/* Extract the search target and send the response as per the search target. */
	tempPtr = os_strcasestr(udpMessage,"\r\nST:");
	if(NULL == tempPtr){
		LOGE("cUPnPDiscovery::VerifyUDPMessageForMSearch Failed as it was unable to find ST string");
		status = FAILURE;
		goto VerifyUDPMessageForMSearch_Exit;
	}else{
		tempPtr += 5;
		while(' ' == *tempPtr){
			tempPtr++;
		}
		tempPtr1 = stString;
		while(' ' != *tempPtr && '\0' != *tempPtr && '\r' != *tempPtr && '\n' != *tempPtr){
			*tempPtr1++ = *tempPtr++;
		}
		/* Now check ST String to send the corresponding NOTIFY packet */
		if(os_strcasestr(stString,"ssdp:all")){
			msearchTarget = MSEARCH_ALL;
		}else if(os_strcasestr(stString,"upnp:rootdevice")){
			msearchTarget = MSEARCH_ROOT;
		}else if(os_strcasestr(stString,"uuid")){
			msearchTarget = MSEARCH_UUID;
		}else if(os_strcasestr(stString,"urn:schemas-upnp-org:device:")){
			msearchTarget = MSEARCH_DEVICE;
			tempPtr1 = stString;
			tempPtr1 += os_strlen((int8*)"urn:schemas-upnp-org:device:");
			os_strncpy(itemType,tempPtr1,sizeof(itemType));
			tempPtr1 = os_strstr(itemType,":");
			*tempPtr1 = '\0';
		}else if(os_strcasestr(stString,"urn:schemas-upnp-org:service:")){
			msearchTarget = MSEARCH_SERVICE;
			tempPtr1 = stString;
			tempPtr1 += os_strlen((int8*)"urn:schemas-upnp-org:service:");
			os_strncpy(itemType,tempPtr1,sizeof(itemType));
			tempPtr1 = os_strstr(itemType,":");
			*tempPtr1 = '\0';
		}
	}
	CreateAndSendMSearchResponse(aUDPSockParam,mxVal,msearchTarget,itemType);
	status = SUCCESS;
	LOGE("cUPnPDiscovery::VerifyUDPMessageForMSearch success and sent M-SEARCH Response");
VerifyUDPMessageForMSearch_Exit:
	/* Free the recv buffer which we got form socket implementation. Otherwise we do see memory leaks. */
	if(aUDPSockParam.RecvBufPtr){
		os_free(aUDPSockParam.RecvBufPtr);
		aUDPSockParam.RecvBufPtr = NULL;
		aUDPSockParam.RecvBufLen = 0;
	}
	return status;
}

ReturnStatus cUPnPDiscovery::VerifyUDPMessageForNotify(IN UDPSocketInfo &aUDPSockParam)
{
	int8 *udpMessage = (int8*)aUDPSockParam.RecvBufPtr,*tempPtr = NULL,*tempPtr1 = NULL;
	int8 deviceDescriptionURL[128] = {'\0'}, uuid[64] = {'\0'}, mxValString[32] = {'\0'};
	uint32 mxVal = 0;
	ReturnStatus status = SUCCESS;
	flag server = false, renderer = false;
	LOGD("cUPnPDiscovery::VerifyUDPMessageForNotify IN for message from: %s",aUDPSockParam.PeerIPPtr);
	
	// sMiLo : if local ip address, ignore it.
	if (os_strncmp(cvLocalAddressPtr, aUDPSockParam.PeerIPPtr, os_strlen(aUDPSockParam.PeerIPPtr)) == 0) {
		LOGD("cUPnPDiscovery::VerifyUDPMessageForNotify IN Skip to add server: %s", aUDPSockParam.PeerIPPtr);
		goto VerifyUDPMessageForNotify_Exit;
	}
	
	if(NULL == os_strcasestr(udpMessage,DIGITAL_MEDIA_SERVER_STRING) && (NULL == os_strcasestr(udpMessage,DIGITAL_MEDIA_RENDERER_STRING))){
		  LOGE("cUPnPDiscovery::VerifyUDPMessageForNotify failed as it was unable to find server string or renderer string in the message");
		  return FAILURE;
	}

	if(NULL == os_strcasestr(udpMessage,"NOTIFY * HTTP/1.1")){
		LOGE("cUPnPDiscovery::VerifyUDPMessageForNotify failed as it was unable to find NOTIFY * HTTP/1.1 in the message");
		status = FAILURE;
		goto VerifyUDPMessageForNotify_Exit;
	}

	tempPtr = os_strcasestr(udpMessage,"\r\nHOST:");
	if(NULL == tempPtr){
		LOGE("cUPnPDiscovery::VerifyUDPMessageForNotify failed as it was unable to find HOST in the message");
		status = FAILURE;
		goto VerifyUDPMessageForNotify_Exit;
	}

	tempPtr += 7;
	while(' ' == *tempPtr || ':' == *tempPtr){
		tempPtr++;
	}

	if(0 != os_strncasecmp(tempPtr,"239.255.255.250",15)){
		LOGE("cUPnPDiscovery::VerifyUDPMessageForNotify failed as it was unable to find 239.255.255.250 in the message");
		status = FAILURE;
		goto VerifyUDPMessageForNotify_Exit;
	}

	tempPtr = os_strcasestr(udpMessage,"\r\nNT:");
	if(NULL == tempPtr){
		LOGE("cUPnPDiscovery::VerifyUDPMessageForNotify failed as it was unable to find NT in the message");
		status = FAILURE;
		goto VerifyUDPMessageForNotify_Exit;
	}

	tempPtr += 4;
	while(' ' == *tempPtr || ':' == *tempPtr){
		tempPtr++;
	}

	if(0 == os_strncasecmp(tempPtr,DIGITAL_MEDIA_SERVER_STRING,os_strlen((int8*)DIGITAL_MEDIA_SERVER_STRING))){
	//if(0 == os_strncasecmp(tempPtr,(int8*)"upnp:rootdevice",15)){
		LOGD("cUPnPDiscovery::VerifyUDPMessageForNotify identified as server from the message");
		server = true;
	}else if(0 == os_strncasecmp(tempPtr,DIGITAL_MEDIA_RENDERER_STRING,os_strlen((int8*)DIGITAL_MEDIA_RENDERER_STRING))){
		LOGD("cUPnPDiscovery::VerifyUDPMessageForNotify identified as renderer from the message");
		renderer = true;
	}else{ /* As of now we are only interested in only MediaServer and MediaRenderes. Other than that for anything else we don't care.... */
		LOGE("cUPnPDiscovery::VerifyUDPMessageForNotify failed as it was unable to identify it as neither server nor renderer");
		goto VerifyUDPMessageForNotify_Exit;
	}

	if(os_strcasestr(udpMessage,"ssdp:alive")){
		LOGD("cUPnPDiscovery::VerifyUDPMessageForNotify message came for ssdp:alive");
		/* Extracting the LOCATION URL From the UDP Message.. */
		tempPtr = os_strcasestr(udpMessage,"LOCATION");
		if(NULL == tempPtr){
			LOGE("cUPnPDiscovery::VerifyUDPMessageForNotify failed as it was unable to find LOCATION in the message");
			status = FAILURE;
			goto VerifyUDPMessageForNotify_Exit;
		}

		tempPtr1 = deviceDescriptionURL;
		tempPtr += 8;
		while(' ' == *tempPtr || ':' == *tempPtr){
			tempPtr++;
		}
		while(' ' != *tempPtr && '\0' != *tempPtr && '\r' != *tempPtr && '\n' != *tempPtr){
			*tempPtr1++ = *tempPtr++;
		}

		/* Extracting the UUID corresponding to this Advertisement... */
		tempPtr = os_strcasestr(udpMessage,"\r\nUSN:");
		if(NULL == tempPtr){
			LOGE("cUPnPDiscovery::VerifyUDPMessageForNotify failed as it was unable to find USN in the message");
			status = FAILURE;
			goto VerifyUDPMessageForNotify_Exit;
		}

		tempPtr += 6;
		while(' ' == *tempPtr || ':' == *tempPtr){
			tempPtr++;
		}

		if(0 != os_strncasecmp(tempPtr,"uuid",4)){
			LOGE("cUPnPDiscovery::VerifyUDPMessageForNotify failed as it was unable to find UUID in the message");
			status = FAILURE;
			goto VerifyUDPMessageForNotify_Exit;
		}

		tempPtr += 4;
		while(' ' == *tempPtr || ':' == *tempPtr){
			tempPtr++;
		}

		tempPtr1 = uuid;
		while(' ' != *tempPtr && '\0' != *tempPtr && '\r' != *tempPtr && '\n' != *tempPtr && ':' != *tempPtr){
			*tempPtr1++ = *tempPtr++;
		}

		/* Extracting the Max Age value until when this Advertisement is valid.... */
		tempPtr = os_strcasestr(udpMessage,"max-age");
		if(NULL == tempPtr){
			LOGE("cUPnPDiscovery::VerifyUDPMessageForNotify failed as it was unable to find max-age in the message");
			status = FAILURE;
			goto VerifyUDPMessageForNotify_Exit;
		}

		tempPtr1 = mxValString;
		tempPtr += 7;
		while(' ' == *tempPtr || '=' == *tempPtr){
			tempPtr++;
		}
		while(' ' != *tempPtr && '\0' != *tempPtr && '\r' != *tempPtr && '\n' != *tempPtr){
			*tempPtr1++ = *tempPtr++;
		}
		if(os_strlen(mxValString)){
			mxVal = os_atoi(mxValString);
		}

		if(server){
			if(cvControPointInterfacePtr){
				LOGD("cUPnPDiscovery:VerifyUDPMessageForNotify adding the server");
				cvControPointInterfacePtr->AddServer(deviceDescriptionURL,uuid,mxVal);
			}
		}else if(renderer){
			if(cvControPointInterfacePtr){
				LOGD("cUPnPDiscovery:VerifyUDPMessageForNotify adding the renderer");
				cvControPointInterfacePtr->AddRenderer(deviceDescriptionURL,uuid,mxVal);
			}
		}
	}else if(os_strcasestr(udpMessage,"ssdp:byebye")){
		LOGD("cUPnPDiscovery::VerifyUDPMessageForNotify message came for ssdp:byebye");
		/* Extracting the UUID corresponding to this Advertisement... */
		tempPtr = os_strcasestr(udpMessage,"\r\nUSN:");
		if(NULL == tempPtr){
			LOGE("cUPnPDiscovery::VerifyUDPMessageForNotify failed as it was unable to find USN in the message");
			status = FAILURE;
			goto VerifyUDPMessageForNotify_Exit;
		}

		tempPtr += 6;
		while(' ' == *tempPtr || ':' == *tempPtr){
			tempPtr++;
		}

		if(0 != os_strncasecmp(tempPtr,"uuid",4)){
			LOGE("cUPnPDiscovery::VerifyUDPMessageForNotify failed as it was unable to find UUID in the message");
			status = FAILURE;
			goto VerifyUDPMessageForNotify_Exit;
		}

		tempPtr += 4;
		while(' ' == *tempPtr || ':' == *tempPtr){
			tempPtr++;
		}

		tempPtr1 = uuid;
		while(' ' != *tempPtr && '\0' != *tempPtr && '\r' != *tempPtr && '\n' != *tempPtr && ':' != *tempPtr){
			*tempPtr1++ = *tempPtr++;
		}

		if(server){
			if(cvControPointInterfacePtr){
				LOGD("cUPnPDiscovery:VerifyUDPMessageForNotify removing the server");
				cvControPointInterfacePtr->RemoveServer(uuid);
			}
		}else if(renderer){
			if(cvControPointInterfacePtr){
				LOGD("cUPnPDiscovery:VerifyUDPMessageForNotify removing the renderer");
				cvControPointInterfacePtr->RemoveRenderer(uuid);
			}
		}

	}else{
		/* Nothing to do with this Advertisement.. Just Ignore... */
		//goto VerifyUDPMessageForNotify_Exit;
	}
	status = SUCCESS;
VerifyUDPMessageForNotify_Exit:
	/* Free the recv buffer which we got form socket implementation. Otherwise we do see memory leaks. */
	if(aUDPSockParam.RecvBufPtr){
		os_free(aUDPSockParam.RecvBufPtr);
		aUDPSockParam.RecvBufPtr = NULL;
		aUDPSockParam.RecvBufLen = 0;
	}
	//LOGD("cUPnPDiscovery:VerifyUDPMessageForNotify OUT. status = %d",status);
	return status;
}

ReturnStatus cUPnPDiscovery::VerifyUDPMessageForMSearchResponse(IN UDPSocketInfo &aUDPSockParam)
{
	int8 *udpMessage = (int8*)aUDPSockParam.RecvBufPtr,*tempPtr = NULL,*tempPtr1 = NULL;
	int8 deviceDescriptionURL[128] = {'\0'}, uuid[64] = {'\0'}, mxValString[32] = {'\0'};
	uint32 mxVal = 0;
	ReturnStatus status = SUCCESS;
	flag server = false, renderer = false;
	LOGD("cUPnPDiscovery::VerifyUDPMessageForMSearchResponse IN for message from: %s",aUDPSockParam.PeerIPPtr);
	
	// sMiLo : if local ip address, ignore it.
	if (os_strncmp(cvLocalAddressPtr, aUDPSockParam.PeerIPPtr, os_strlen(aUDPSockParam.PeerIPPtr)) == 0) {
		LOGD("cUPnPDiscovery::VerifyUDPMessageForMSearchResponse IN Skip to add server: %s", aUDPSockParam.PeerIPPtr);
		goto VerifyUDPMessageForMSearchResponse_Exit;
	}
	
	if(NULL == os_strcasestr(udpMessage,"HTTP/1.1 200 OK")){
		LOGE("cUPnPDiscovery::VerifyUDPMessageForMSearchResponse failed as it was unable to find HTTP/1.1 200 OK in the message");
		status = FAILURE;
		goto VerifyUDPMessageForMSearchResponse_Exit;
	}

	if(os_strcasestr(udpMessage,DIGITAL_MEDIA_SERVER_STRING)){
		LOGD("cUPnPDiscovery::VerifyUDPMessageForMSearchResponse identified as server");
		server = true;
	}else if(os_strcasestr(udpMessage,DIGITAL_MEDIA_RENDERER_STRING)){
		LOGD("cUPnPDiscovery::VerifyUDPMessageForMSearchResponse identified as renderer");
		renderer = true;
	}else{ /* As of now we are only interested in only MediaServer and MediaRenderes. Other than that for anything else we don't care.... */
		LOGE("cUPnPDiscovery::VerifyUDPMessageForMSearchResponse identified as server");
		goto VerifyUDPMessageForMSearchResponse_Exit;
	}

	/* Extracting the LOCATION URL From the UDP Message.. */
	tempPtr = os_strcasestr(udpMessage,"\r\nLOCATION");
	if(NULL == tempPtr){
		LOGE("cUPnPDiscovery::VerifyUDPMessageForMSearchResponse failed as it was unable to find LOCATION in the message");
		status = FAILURE;
		goto VerifyUDPMessageForMSearchResponse_Exit;
	}

	tempPtr1 = deviceDescriptionURL;
	tempPtr += 10;
	while(' ' == *tempPtr || ':' == *tempPtr){
		tempPtr++;
	}
	while(' ' != *tempPtr && '\0' != *tempPtr && '\r' != *tempPtr && '\n' != *tempPtr){
		*tempPtr1++ = *tempPtr++;
	}

	/* Extracting the UUID corresponding to this Advertisement... */
	tempPtr = os_strcasestr(udpMessage,"\r\nUSN:");
	if(NULL == tempPtr){
		LOGE("cUPnPDiscovery::VerifyUDPMessageForMSearchResponse failed as it was unable to find USN in the message");
		status = FAILURE;
		goto VerifyUDPMessageForMSearchResponse_Exit;
	}

	tempPtr += 6;
	while(' ' == *tempPtr || ':' == *tempPtr){
		tempPtr++;
	}

	if(0 != os_strncasecmp(tempPtr,"uuid",4)){
		LOGE("cUPnPDiscovery::VerifyUDPMessageForMSearchResponse failed as it was unable to find UUID in the message");
		status = FAILURE;
		goto VerifyUDPMessageForMSearchResponse_Exit;
	}

	tempPtr += 4;
	while(' ' == *tempPtr || ':' == *tempPtr){
		tempPtr++;
	}

	tempPtr1 = uuid;
	while(' ' != *tempPtr && '\0' != *tempPtr && '\r' != *tempPtr && '\n' != *tempPtr && ':' != *tempPtr){
		*tempPtr1++ = *tempPtr++;
	}
	/* Extracting the Max Age value until when this Advertisement is valid.... */
	tempPtr = os_strcasestr(udpMessage,"max-age");
	if(NULL == tempPtr){
		LOGE("cUPnPDiscovery::VerifyUDPMessageForMSearchResponse failed as it was unable to find max-age in the message");
		status = FAILURE;
		goto VerifyUDPMessageForMSearchResponse_Exit;
	}
	tempPtr += 7;
	tempPtr1 = mxValString;
	while(' ' == *tempPtr || '=' == *tempPtr){
		tempPtr++;
	}
	while(' ' != *tempPtr && '\0' != *tempPtr && '\r' != *tempPtr && '\n' != *tempPtr){
		*tempPtr1++ = *tempPtr++;
	}
	if(os_strlen(mxValString)){
		mxVal = os_atoi(mxValString);
	}

	if(server){
		if(cvControPointInterfacePtr){
			LOGD("cUPnPDiscovery::VerifyUDPMessageForMSearchResponse adding the server");
			cvControPointInterfacePtr->AddServer(deviceDescriptionURL,uuid,mxVal);
		}
	}else if(renderer){
		if(cvControPointInterfacePtr){
			LOGD("cUPnPDiscovery::VerifyUDPMessageForMSearchResponse adding the renderer");
			cvControPointInterfacePtr->AddRenderer(deviceDescriptionURL,uuid,mxVal);
		}
	}

	status = SUCCESS;
VerifyUDPMessageForMSearchResponse_Exit:
	/* Free the recv buffer which we got form socket implementation. Otherwise we do see memory leaks. */
	if(aUDPSockParam.RecvBufPtr){
		os_free(aUDPSockParam.RecvBufPtr);
		aUDPSockParam.RecvBufPtr = NULL;
		aUDPSockParam.RecvBufLen = 0;
	}
	return status;
}
/* Virtual Function Definitions from cSocketObserver */
void cUPnPDiscovery::HandleSocketActivity(IN SocketObserverParams &aSockObsParms)
{
	if(UDP_MESSAGE_RECEIVED == aSockObsParms.SockActivity && NULL != aSockObsParms.UDPSockInfoPtr){
		//LOG_CONSOLE("cUPnPDiscovery::HandleSocketActivity Callback Came!!");
		/* Here check for the UDP Message and call the correspoinding function. In case of Server we are interested in M-SEARCH Messages and incase of Control Point we are interested in NOTIFY Messages. */
		int8 *udpMessage = (int8*)((UDPSocketInfo*)aSockObsParms.UDPSockInfoPtr)->RecvBufPtr;
		if(true == cvDiscoveryForServer){
			if(os_strcasestr(udpMessage,"M-SEARCH")){
				//LOGD("cUPnPDiscovery::HandleSocketActivity message came with M-SEARCH from: %s",aSockObsParms.UDPSockInfoPtr->PeerIPPtr);
				VerifyUDPMessageForMSearch(*aSockObsParms.UDPSockInfoPtr);
			}else{
				if(udpMessage){
					os_free(udpMessage);
				}
			}
		}else{
			if(os_strcasestr(udpMessage,"NOTIFY")){
				//LOGD("cUPnPDiscovery::HandleSocketActivity message came with NOTIFY from: %s",aSockObsParms.UDPSockInfoPtr->PeerIPPtr);
				VerifyUDPMessageForNotify(*aSockObsParms.UDPSockInfoPtr);
			}else if(os_strcasestr(udpMessage,"HTTP/1.1 200 OK")){
				//LOGD("cUPnPDiscovery::HandleSocketActivity message came with HTTP/1.1 200 OK from: %s",aSockObsParms.UDPSockInfoPtr->PeerIPPtr);
				VerifyUDPMessageForMSearchResponse(*aSockObsParms.UDPSockInfoPtr);
			}else{
				if(udpMessage){
					os_free(udpMessage);
					udpMessage = NULL;
				}
			}
		}
	}
}

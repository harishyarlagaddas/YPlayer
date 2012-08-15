#include "upnp_base.h"

int32 cUPnPBase::giNumOfActiveIPAddr = 0;
int8 cUPnPBase::gcaActiveIPAddr[20][MAXIMUM_IP_SIZE] = {{'\0'}};

cUPnPBase::cUPnPBase()
{
	//os_memset(&cvSSDPInfoStruct,'\0',sizeof(cvSSDPInfoStruct));
	//os_memset(&cvRootDeviceInfoStruct,'\0',sizeof(cvRootDeviceInfoStruct));
}

cUPnPBase::~cUPnPBase()
{
	if(GetEnbedDeviceList()->GetNumberOfElements()){
		GetEnbedDeviceList()->DestroyList();
	}
	if(GetServiceIds()->GetNumberOfElements()){
		GetServiceIds()->DestroyList();
	}
	if(GetDeviceIconList()->GetNumberOfElements()){
		GetDeviceIconList()->DestroyList();
	}
}

void cUPnPBase::SetSSDPInfo(const SSDPInfoStruct& aDiscoveryStruct)
{
	os_memcpy(GetSSDPInfo(),&aDiscoveryStruct,sizeof(SSDPInfoStruct));
}

void cUPnPBase::SetRootDeviceInfo(const SSDPDeviceInfoStruct& aRootDeviceStruct)
{
	os_memcpy(GetRootDeviceInfo(),&aRootDeviceStruct,sizeof(SSDPDeviceInfoStruct));
}

void cUPnPBase::AddEmbedDeviceInfo(SSDPDeviceInfoStruct& aEmbdDeviceStruct)
{
	GetEnbedDeviceList()->Add(aEmbdDeviceStruct);
}

void cUPnPBase::AddService(SSDPServiceID aServiceID)
{
	GetServiceIds()->Add(aServiceID);
}

void cUPnPBase::AddDeviceIcon(SSDPDeviceIconStruct& aDevIconStruct)
{
	GetDeviceIconList()->Add(aDevIconStruct);
}

SSDPInfoStruct* cUPnPBase::GetSSDPInfo()
{
	/* Static variable used to hold SSDP core Information set using the SetSSDPInfo api */
	static SSDPInfoStruct ssdpInfoStruct;

	return &ssdpInfoStruct;
}

SSDPDeviceInfoStruct* cUPnPBase::GetRootDeviceInfo()
{

	/* static variable used to hold Root device Information set using the SetRootDeviceInfo api */
	static SSDPDeviceInfoStruct rootDeviceInfoStruct;

	return &rootDeviceInfoStruct;
}

cLinkedList<SSDPDeviceInfoStruct>* cUPnPBase::GetEnbedDeviceList()
{

	/* This is the list which contains info about each embeded device added using AddEmbedDeviceInfo */
	static cLinkedList<SSDPDeviceInfoStruct> embdDeviceInfoList;

	return &embdDeviceInfoList;
}

cLinkedList<SSDPServiceID>* cUPnPBase::GetServiceIds()
{
	/* This is the list which contains info about each service added using AddServiceInfo */
	static cLinkedList<SSDPServiceID> supportedServiceIDs;

	return &supportedServiceIDs;
}

cLinkedList<SSDPDeviceIconStruct>* cUPnPBase::GetDeviceIconList()
{
	/* This is the list which contains info about each device icon added using AddDeviceIcon */
	static cLinkedList<SSDPDeviceIconStruct> deviceIconList;

	return &deviceIconList;
}

/* Static Function Definitions to get the Active Ethernet Addresses. */
void cUPnPBase::RefreshAtiveIPAddress()
{
	int32 i=0, j=0;
	int8 cLocalAddressFound = 0;
	int8 *cpAddressPtr = NULL;
	cUPnPBase::giNumOfActiveIPAddr = os_get_NumberOfActiveEthernetInterfaces();
	LOGD("cUPnPBase::RefreshAtiveIPAddress Number of ActiveEthernetInterfaces:[%d]",cUPnPBase::giNumOfActiveIPAddr);
	if(cUPnPBase::giNumOfActiveIPAddr <= 0){
		LOGE("cUPnPBase::RefreshAtiveIPAddress Number of Active IP Addresses is less than zero");
		return;
	}
	cpAddressPtr = (int8*)os_malloc(cUPnPBase::giNumOfActiveIPAddr * MAXIMUM_IP_SIZE);
	if(cpAddressPtr == NULL){
		LOGE("cUPnPBase::RefreshAtiveIPAddress Failed to allocate the Memory");
		return;
	}
	os_memset(cpAddressPtr,'\0',(cUPnPBase::giNumOfActiveIPAddr * MAXIMUM_IP_SIZE));
	os_get_ActiveEthernetInterfaces(cpAddressPtr,(cUPnPBase::giNumOfActiveIPAddr * MAXIMUM_IP_SIZE));
	for(i=0,j=0; i< cUPnPBase::giNumOfActiveIPAddr; i++){
	  if(strcmp(cpAddressPtr+(i*MAXIMUM_IP_SIZE), "127.0.0.1") != 0){
		os_strncpy(cUPnPBase::gcaActiveIPAddr[j],cpAddressPtr+(i*MAXIMUM_IP_SIZE),sizeof(cUPnPBase::gcaActiveIPAddr[j]));
		//UPnP_BASE_LOG_DEBUG("cUPnPBase::RefreshAtiveIPAddress copying the IP to Globla arrary gcaActiveIPAddr[%d]:%s",
		//	   j,cUPnPBase::gcaActiveIPAddr[j]);
		j++;
	  }else{
		LOGD("cUPnPBase::RefreshAtiveIPAddress localhost address found 127.0.0.1");
#ifdef SSDP_LOCAL_HOST_IP_ENABLE
		os_strncpy(cUPnPBase::gcaActiveIPAddr[j],cpAddressPtr+(i*MAXIMUM_IP_SIZE),sizeof(cUPnPBase::gcaActiveIPAddr[j]));
		//UPnP_BASE_LOG_DEBUG("cUPnPBase::RefreshAtiveIPAddress copying the IP to Global arrary gcaActiveIPAddr[%d]:%s",
		//	   j,cUPnPBase::gcaActiveIPAddr[j]);
		j++;
#endif
		cLocalAddressFound = 1;
	    }
	}
	os_free(cpAddressPtr);
#ifndef SSDP_LOCAL_HOST_IP_ENABLE
	if(cLocalAddressFound) cUPnPBase::giNumOfActiveIPAddr--; //Removing the count for 127.0.0.1 localhost.
#endif
	i=0; cpAddressPtr = NULL; cLocalAddressFound = 0;
}
int32 cUPnPBase::GetNumOfActiveIPAddr()
{
	return cUPnPBase::giNumOfActiveIPAddr;
}

int8* cUPnPBase::GetActiveIPAddrAtIndex(const int32 aNum)
{
	if(aNum > cUPnPBase::giNumOfActiveIPAddr){
		LOGE("cUPnPBase::GetActiveIPAddrAtIndex OUT Returning NULL as incomming index is more than valid interfaces");
		return NULL;
	}
	if(os_strlen(cUPnPBase::gcaActiveIPAddr[aNum-1])){
		LOGD("cUPnPBase::GetActiveIPAddrAtIndex OUT Returning %s",cUPnPBase::gcaActiveIPAddr[aNum-1]);
		return cUPnPBase::gcaActiveIPAddr[aNum-1];
	}
	return NULL;
}

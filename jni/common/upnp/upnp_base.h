#ifndef UPNP_BASE_H_INCLUDED
#define UPNP_BASE_H_INCLUDED

#include "upnp_datatypes.h"
#include "../../os/os.h"
#include "../include/return_codes.h"
#include "../log/log.h"
#include "../socketimpl/socketimpl.h"
#include "../util/linkedlist.h"
#include "../protocols/http/http_utils.h"

typedef enum{
	UPNP_GUID = 0,
	ROOT,
	DEVICE,
	SERVICE
}cvType;

/* cUPnPBase: This class is the base class for UPnP stack. This class will do nothing except holding the necessary information
   about the devices, embeded devices, services and device icons present inside the system. Actually Device Discovery and
   device description classes will be derived this class such that the info set inside this class by the application will
   be used by then to send Discovery and Description messages on to the network. For the detailed description of the each
   api please see below where it is explained near each api.
*/
class cUPnPBase
{
public:
	cUPnPBase();
	virtual ~cUPnPBase();

	/* SetSSDPInfo - This function will be used set the SSDP information by the upnp application. First the application
	   needs to initialize this info and after that it can start UPnP operations. Actually the information set in this
	   class including this function will be used in UPnP Discovery and Discription messages. With out setting this info
	   starting the UPnP operations will give the un expected behaviour.

	  Note: Application must and should set this SSDP Info by using this fucntion before start of any UPnP Operation.

	   Input Parameters:
	   param1: const SSDPInfoStruct&: SSDPInfoStruct which contains the variables to hold the necessary SSDP information.

	   Return Value:
	   Void - This is the void function. Will return nothing.
	*/
	void SetSSDPInfo(const SSDPInfoStruct&);

	/* SetRootDeviceInfo - This function will be used set the information about the Root device by the upnp application.
	   First the application needs to set this info and after that it can start UPnP operations. Actually the information
	   set in this class including this function will be used in UPnP Discovery and Discription messages. With out
	   setting this info starting the UPnP operations will give the un expected behaviour.

	  Note: Application must and should set this Root Device Info by using this fucntion before start of any UPnP
	       Operation. Even if we think with out root device info it makes no sense to start UPnP operations.

	   Input Parameters:
	   param1: const SSDPDeviceInfoStruct&: SSDPDeviceInfoStruct which contains the variables to hold the necessary
	   information about the root device.

	   Return Value:
	   Void - This is the void function. Will return nothing.
	*/
	void SetRootDeviceInfo(const SSDPDeviceInfoStruct&);

	/* AddEmbedDeviceInfo - This function will be used set the information about the embedded devices if they present by
	  the upnp application. If any embedded devices are present then the application needs to add the info about each
	  device by calling this api as many times as number of embedded devices.Actually the information
	   set in this class including this function will be used in UPnP Discovery and Discription messages. With out
	   setting this info no embeded devices will be advertised in the UPnP Discovery and Description messages.

	  Note: Application must and should set this Embedded Device Info by using this fucntion if it has any embedded
	  devices. If no embeded devices are present then there is no use of calling this api. If embeded devices are
	  present and even then if this api is not called then the UPnP Discovery and Description messages won't contain
	  any information about the Embedded Device info that is not added here.

	   Input Parameters:
	   param1: const SSDPDeviceInfoStruct&: SSDPDeviceInfoStruct which contains the variables to hold the necessary
	   information about the each embedded device.

	   Return Value:
	   Void - This is the void function. Will return nothing.
	*/
	void AddEmbedDeviceInfo(SSDPDeviceInfoStruct&);

	/* AddService - This function will be used set the information about each of the services supported by
	  the upnp application. If any services are supported then the application needs to add the info about each
	  service by calling this api as many times as number of services.Actually the information
	   set in this class including this function will be used in UPnP Discovery and Discription messages. With out
	   setting this info no services will be advertised in the UPnP Discovery and Description messages.

	  Note: Application must and should set this serviceID  by using this fucntion if it supports any of the
	  services. If no services are supported then there is no use of calling this api. If services are actually supported
	  and even then if this api is not called then the UPnP Discovery and Description messages won't contain
	  any information about the service that is not added here.

	   Input Parameters:
	   param1: SSDPServiceID: SSDPServiceID which contains the ServiceID of the supported service.

	   Return Value:
	   Void - This is the void function. Will return nothing.
	*/
	void AddService(SSDPServiceID);

	/* AddDeviceIcon - This function will be used set the information about Device Icon if it has any. Actually during
	   device description we used to send the info about the device icon. We also can send the info about multiple
	   device icons. So for each device icon it has application needs to add it using this api

	  Note: If application add the info about dev icon then only this info will be sent in the device description.If
	  it is not added here then no information will be sent in the device description.

	   Input Parameters:
	   param1: const SSDPDeviceIconStruct&: SSDPDeviceIconStruct which contains the variables to hold the necessary
	   information about device icon

	   Return Value:
	   Void - This is the void function. Will return nothing.
	*/
	void AddDeviceIcon(SSDPDeviceIconStruct&);

	/* GetSSDPInfo - This function will be used by the UPnPDiscovery class and UPnPDescription class to retrieve the
	   pointer to SSDPInfoStruct where all the information about SSDP is set using SetSSDPInfo api

	   Input Parameters:
	   Void - This takes no parametes.

	   Return Value:
	   SSDPInfoStruct* - Pointer to the SSDPInfoStruct where the information about SSDP is set using SetSSDPInfo api
	*/
	static SSDPInfoStruct* GetSSDPInfo();

	/* GetRootDeviceInfo - This function will be used by the UPnPDiscovery class and UPnPDescription class to retrieve the
	   pointer to SSDPDeviceInfoStruct where all the information about root device is set using SetRootDeviceInfo api

	   Input Parameters:
	   Void - This takes no parametes.

	   Return Value:
	   SSDPInfoStruct* - Pointer to the SSDPDeviceInfoStruct where the information about root device is set using
	   SetRootDeviceInfo api
	*/
	static SSDPDeviceInfoStruct* GetRootDeviceInfo();

	/* GetEnbedDeviceList - This function will be used by the UPnPDiscovery class and UPnPDescription class to retrieve the
	   pointer to SSDPDeviceInfoStruct linked list where all the information about each embedded device is set
	  using AddEmbedDeviceInfo api

	   Input Parameters:
	   Void - This takes no parametes.

	   Return Value:
	   SSDPInfoStruct* - Pointer to the SSDPDeviceInfoStruct linked list where the information about each embedded device
	   is set using AddEmbedDeviceInfo api
	*/
	static cLinkedList<SSDPDeviceInfoStruct>* GetEnbedDeviceList();

	/* GetServiceList - This function will be used by the UPnPDiscovery class and UPnPDescription class to retrieve the
	   pointer to SSDPServiceInfoStruct linked list where all the information about each service is set
	  using AddServiceInfo api

	   Input Parameters:
	   Void - This takes no parametes.

	   Return Value:
	   SSDPServiceID* - Pointer to the SSDPServiceID linked list where each serviceid is added by using AddServiceInfo api
	*/
	static cLinkedList<SSDPServiceID>* GetServiceIds();

	/* GetDeviceIconList - This function will be used by the UPnPDiscovery class and UPnPDescription class to retrieve the
	   pointer to SSDPDeviceIconStruct linked list where all the information about each device icon is set
	  using AddDeviceIcon api

	   Input Parameters:
	   Void - This takes no parametes.

	   Return Value:
	   SSDPInfoStruct* - Pointer to the SSDPDeviceIconStruct linked list where the information about each service
	   is set using AddDeviceIcon api
	*/
	static cLinkedList<SSDPDeviceIconStruct>* GetDeviceIconList();

	static void RefreshAtiveIPAddress();
	static int32 GetNumOfActiveIPAddr();
	static int8* GetActiveIPAddrAtIndex(const int32 aNum);
private:
	static int32 giNumOfActiveIPAddr;
	static int8 gcaActiveIPAddr[20][MAXIMUM_IP_SIZE];

};
#endif /* UPNP_BASE_H_INCLUDED */

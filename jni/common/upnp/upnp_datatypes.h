#ifndef UPNP_DATATYPES_H_INCLUDED
#define UPNP_DATATYPES_H_INCLUDED

#include "../include/basic_datatypes.h"
#include "../../os/os.h"
#include "../util/linkedlist.h"

/* Strings used at Various places inside UPnP Stack for Various purposes. */
#define DEVICE_DESCRIPTION_STRING "/DeviceDescription.xml"

/* Strings used by the Control Point to search for the devices on the network */
#define DIGITAL_MEDIA_SERVER_STRING "urn:schemas-upnp-org:device:MediaServer:1"
#define DIGITAL_MEDIA_RENDERER_STRING "urn:schemas-upnp-org:device:MediaRenderer:1"

/* Strings related the Content Directory Service. */
#define CONTENT_DIRECTORY_SERVICE_TYPE "urn:schemas-upnp-org:service:ContentDirectory:1"
#define CONTENT_DIRECTORY_SERVICE_ID_NAME "urn:upnp-org:serviceId:ContentDirectory"
#define CDS_DESCRIPTION_STRING "/cds.xml"
#define CDS_CONTROLLING_STRING "/cds/control"
#define CDS_EVENTING_STRING "/cds/event"

/* Strings related the Connection Manager Service. */
#define CONNECTION_MANAGER_SERVICE_TYPE "urn:schemas-upnp-org:service:ConnectionManager:1"
#define CONNECTION_MANAGER_SERVICE_ID_NAME "urn:upnp-org:serviceId:ConnectionManager"
#define CMS_DESCRIPTION_STRING "/cms.xml"
#define CMS_CONTROLLING_STRING "/cms/control"
#define CMS_EVENTING_STRING "/cms/event"

/* Strings related to the Content retrieval. These will be include while constructing the content URL. */
#define BASIC_CONTENT_URL_STRING 	"/dlna/get_content/"
#define IMAGE_THUMBNAIL_STRING		"thumbnail/"

/* Strings related to supported DLNA operations which will be added in Device Description according to the value set by Applications. */
#define DLNA_DMS_SUPPORTED_STRING "DMS-1.50"
#define DLNA_DMR_SUPPORTED_STRING "DMR-1.50"
#define DLNA_MDMS_SUPPORTED_STRING "M-DMS-1.50"

/**************Structures/Enums to specify which DLNA operations are supported****************/
typedef enum{
	DLNA_NOT_SUPPORTED = 0x0001,
	DLNA_DMS_SUPPORTED = 0x0010,
	DLNA_DRMR_SUPPORTED= 0x0100,
	DLNA_MDMS_SUPPORTED= 0x1000
}SupportedDLNAOps;

/***************** SSDP Discovery Related Structures *************************/
typedef struct{
	int8 SSDPBroadcastAddr[MAXIMUM_IP_SIZE]; // SSDP Broadcast address. Generally it will be "239.255.255.250".
	uint32 SSDPBroadcastPortNo; // SSDP Broadcast port no. Generally it will be 1900
	uint32 SSDPDiscoveryMsgLifetime; // Life time of the SSDP Discovery packet. Before expiry NOTIFY packets needs to be sent again.
	int8 SSDPOSName[64]; // Name of the Operating System on which this stack is running.
	int32 SSDPOSMajorVersionNo; // Operating System Major Version Number. Used while composing NOTIFY packets.
	int32 SSDPOSMinorVersionNo; // Operating System Minor Version Number. Used while composing NOTIFY packets.
	int8 SSDPProductName[64]; // Name of the Product. Will be notified in discovery messages.
	int32 SSDPProductMajorVersionNo; // Product Major Version Number. will be notified in discovery messages.
	int32 SSDPProductMinorVersionNo; // Product Minor Version Number. will be notified in discovery messages.
	/* This denotes the Device Description String & port which will be appended to the URL and sent in Discovery Messages. Control point will request on this
	   URL for requesting the Device Description of each embedded device, service, e.t.c.. */
	int32 SSDPDDPort;
}SSDPInfoStruct;

typedef struct{
	int8 DeviceName[64]; // Name of the device (no matter of Root device or Embedded device.
	int8 DeviceUUID[64]; // Device GUID. Sent in Discovery messages of the devices.
	int8 DeviceFriendlyName[64]; // This field will be used in Device Desription.
	int8 ManufacturerName[64]; // This field will be used in Device Desription.
	int8 ManufacturerURL[128]; // This field will be used in Device Desription.
	int8 ModelDescription[128]; // This field will be used in Device Desription.
	int8 ModelName[32]; // This field will be used in Device Desription.
	int8 ModelNumber[32]; // This field will be used in Device Desription.
	int8 ModelURL[128]; // This field will be used in Device Desription.
	int8 SerialNumber[64]; // This field will be used in Device Desription.
	int8 ProductCode[12]; // This field will be used in Device Desription.
	int32 DLNAOps; // This field specified which DLNA operations are supported and the fields are added inside Device Description according to this value.
}SSDPDeviceInfoStruct;

typedef enum{
	CMS = 0,
	CONTENT_DIRECTORY_SERVICE_ID = 1,
	CONNECTION_MANAGER_SERVICE_ID
}SSDPServiceID;

typedef struct{
	  int8 Mimetype[64]; //Mime type of the image.
	  int32 Width; // Width of the image.
	  int32 Height; // Height of the image.
	  int32 Depth; // Depth of the image.
	  int8 URL[128]; //URL to the icon by which any one can get it using HTTP GET request.
}SSDPDeviceIconStruct;
/***************** SSDP Discovery Related Structures END. *************************/

/***************** UPnP Control Point Related Structures BEGIN. *************************/
typedef struct{
	int32 ServiceID; // Name of the Service
	int8 SCPDURL[64]; //URL where we can get the serviceinfo.xml..
	int8 ControlURL[64]; // URL on which we will get the service related requests
	int8 EventingURL[64]; // URL on which evening related requests will go
}ServiceInfo;

typedef struct{
	int8 DeviceName[64]; // Name of the device which will come in to the network.
	int8 DeviceStringInURL[64];
	int32 deviceID;
	int8 BaseURL[64]; //Base URL of the device..
	int8 DeviceIP[64]; // Device IP Address
	int32 DevicePortNo;
	int8 DeviceUUID[64];
	int32 ValidDuration;
	os_time_struct LastActiveTime;
	cLinkedList<ServiceInfo> *ServiceList;
}DeviceInfo;

/***************** UPnP Control Point Related Structures END. *************************/
#endif /* UPNP_DATATYPES_H_INCLUDED */

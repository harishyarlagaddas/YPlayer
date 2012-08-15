#ifndef UPNP_DISCOVERY_H_INCLUDED
#define UPNP_DISCOVERY_H_INCLUDED

#include "../include/basic_datatypes.h"
#include "../../os/os.h"
#include "../include/return_codes.h"
#include "../log/log.h"
#include "../socketimpl/socketimpl.h"
#include "../util/linkedlist.h"
#include "../upnp/upnp_base.h"
//#include "upnp_control_point.h"

/* Macros related to time interval mentioned in the Broadcast messages.. */
#define UPnP_DISCOVERY_MX_TIME_INTEVAL_IN_M_SEARCH	5


class UPnPControlPointInterface
{
  public:
	UPnPControlPointInterface() {};
	virtual ~UPnPControlPointInterface() {};

	virtual ReturnStatus AddServer(int8 *aUrl, int8* aUUID, int32 aLifeTime) = 0;
	virtual ReturnStatus AddRenderer(int8 *aUrl, int8* aUUID, int32 aLifeTime) = 0;

	virtual ReturnStatus RemoveServer(int8* aUUID) = 0;
	virtual ReturnStatus RemoveRenderer(int8* aUUID) = 0;
};

class cUPnPDiscovery : public cUPnPBase ,  public cSocketObserver
{
public:
	cUPnPDiscovery();
	~cUPnPDiscovery();

	void StartUPnPDiscovery();
	void StopUPnPDiscovery();

	void SetDiscoveryForServer(void);
	void SetDiscoveryForControlPoint(void);
	void RegisterControlPointInterfacePtr(UPnPControlPointInterface *aCPIntrerfacePtr);
	/* Virtual Functions from cSocketObserver */
	void HandleSocketActivity(IN SocketObserverParams &aSockObsParms);
private:
	typedef enum{
		NOTIFY_GUID = 1,
		NOTIFY_ROOT,
		NOTIFY_DEVICE,
		NOTIFY_SERVICE,
		MSEARCH_ALL,
		MSEARCH_ROOT,
		MSEARCH_UUID,
		MSEARCH_DEVICE,
		MSEARCH_SERVICE
	}cvUDPPacketType;

	typedef enum{
		ALL = 1,
		ROOT_DEVICE,
		DEVICE_UUID,
		DEVICE_TYPE,
		SERVICE_TYPE,
		MEDIA_SERVER, // This will be used by Control Point to search for the media servers on the network.
		MEDIA_RENDERER // This will be used by the Control Point to search for the media renderers on the network.
	}MsearchTarget;

	typedef struct{
		SOCKET BroadCastSocketID;
		SOCKET ListenSocketID;
		cSocket* BroadCastSocket;
		cSocket* ListenSocket;
		int8 InterfaceIPAddr[MAXIMUM_IP_SIZE];
	}InterfaceSockParmStruct;

	void SendNotifyPackets(InterfaceSockParmStruct* aIFSockParmStruct);
	void CreateNotifyPacket(IN const cvUDPPacketType aType,IN const void* vpInStruct, IN const int8* aDDIP, OUT int8* aOutBuf,
					IN const int32 aOutBufLen);

	void SendMSearchPackets(IN InterfaceSockParmStruct* aIFSockParmStruct);
	void CreateMSearchPacket(IN const MsearchTarget aType, INOUT int8* aOutBuf, IN const int32 aOutBufLen);

	void LogSSDPMessages(int8* cpInBuf);
	void SendMSearchResponse(IN InterfaceSockParmStruct* aIFSockParmStruct,IN UDPSocketInfo &aUDPSockParam,
				 cvUDPPacketType aMsearchTarget,const int8* aItemName);
	ReturnStatus CreateAndSendMSearchResponse(IN UDPSocketInfo &aUDPSockParam,uint32 aMXVal,
						  cvUDPPacketType aMsearchTarget,const int8* aItemName);
	ReturnStatus VerifyUDPMessageForMSearch(IN UDPSocketInfo &aUDPSockParam);
	ReturnStatus VerifyUDPMessageForNotify(IN UDPSocketInfo &aUDPSockParam);
	ReturnStatus VerifyUDPMessageForMSearchResponse(IN UDPSocketInfo &aUDPSockParam);

	void SendByeByePackets();
	void CreateByeByePacket(IN const cvUDPPacketType aType, IN const int8* aTargetName, OUT int8* aOutBuf,
				IN const int32 aOutBufLen);

	static int32 giNumOfActiveIPAddr;
	static int8 gcaActiveIPAddr[20][MAXIMUM_IP_SIZE];

	/* Flag to indicate for which purpose this discovery module is being used. (1. Server 2. Control Point) */
	flag cvDiscoveryForServer;

	cLinkedList<InterfaceSockParmStruct> cvInterfaceSockParmStruct;
	flag cvDiscoveryActive;
	static os_semaphore cvControlPointSem;
	/* Control Point interface Pointer using which we can callback the functions for intimation about the addition of the server or renderer */
	UPnPControlPointInterface *cvControPointInterfacePtr;
};
#endif /* UPNP_DISCOVERY_H_INCLUDED */

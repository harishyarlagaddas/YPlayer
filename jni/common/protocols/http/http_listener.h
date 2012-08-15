
#ifndef HTTP_LISTENER_H_INCLUDED
#define HTTP_LISTENER_H_INCLUDED

#include "../../include/basic_datatypes.h"
#include "../../../os/os.h"
#include "../../include/return_codes.h"
#include "../../log/log.h"
#include "../../socketimpl/socketimpl.h"
#include "../../util/linkedlist.h"
#include "http_utils.h"

class cHTTPListenerInterface
{
public:
	cHTTPListenerInterface(){};
	virtual ~cHTTPListenerInterface(){};
	virtual void HeaderReceived(IN HttpInfo& aHttpInfo) = 0;
};

typedef struct{
	HttpRequestType RequestType;
	int8 MatchingString[256];
}RequestData;

typedef struct {
	RequestData Data;
	cHTTPListenerInterface *Observer;
}ObserverData;

typedef enum{
	GETCOUNT = 1,
	GETOBSERVER,
	SETOBSERVER,
	DELETEOBSERVER /*DELETE is windows definition,please use appropriate*/
}ObserverAction;

#define HTTP_LISTEN_MAX_SOCKET_ID_COUNT  256

/* Logging Macros which can be used inside UPnP Base class to get the selective logging. */
#define HTTP_LISTENER_LOG_TRACE if(cvLogPtr) LOG_TRACE
#define HTTP_LISTENER_LOG_DEBUG if(cvLogPtr) LOG_DEBUG
#define HTTP_LISTENER_LOG_ERROR if(cvLogPtr) LOG_ERROR

class cHTTPListener : public cSocketObserver
{
public:
	cHTTPListener();
	~cHTTPListener();

	ReturnStatus CreateHTTPListenSocket(IN const int8* const aAddr,IN const uint32 aPort);
	/* Virtual Functions from cSocketObserver */
	void HandleSocketActivity(IN SocketObserverParams &aSockObsParms);

	static void RegisterObserver(IN HttpRequestType aHttpRequestType, IN const int8 *aMatchingString,
				     IN cHTTPListenerInterface *aObserver);

	static ReturnStatus ReceiveAndParseHeader(IN HttpInfo &aHttpInfo);

	void CloseAllConnections();
private:
	static ReturnStatus ActionOnObserver(IN ObserverAction aActionOnObserver,INOUT uint32 *aCount=NULL,INOUT void *aInputPtr=NULL, OUT void **aOutPtr=NULL);

	SOCKET cvSockID;
	cSocket cvSocket;

	SOCKET cvActiveSockIDs[HTTP_LISTEN_MAX_SOCKET_ID_COUNT];
	int8 cvOwnIP[MAXIMUM_IP_SIZE];
	uint32 cvOwnPort;
};
#endif /* HTTP_LISTENER_H_INCLUDED */


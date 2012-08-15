#ifndef HTTP_UTILS_H_INCLUDED
#define HTTP_UTILS_H_INCLUDED

#include "../../include/basic_datatypes.h"
#include "../../../os/os.h"
#include "../../include/return_codes.h"
#include "../../log/log.h"
#include "../../socketimpl/socketimpl.h"
#include "../../util/linkedlist.h"
#include "../../upnp/upnp_base.h"

/***************** HTTP Related Structures *************************/
#define HTTP_HEADER_SIZE 1024
#define HTTP_REQUEST_SIZE 1024

typedef enum{
	HTTP_GET = 0,
	HTTP_POST,
	HTTP_HEAD,
	HTTP_NONE
}HttpRequestType;

typedef enum{
	HTTP_0_0 = 0,
	HTTP_1_0,
	HTTP_1_1
}HttpVersion;

typedef enum{
	HTTP_RESPONSE_OK,
	HTTP_RESPONSE_BAD_REQUEST,
	HTTP_RESPONSE_NOT_FOUND,
	HTTP_RESPONSE_NONE
}HttpResponseCode;

typedef enum{
	CONNECTION_KEEP_ALIVE,
	CONNECTION_CLOSE
}ConnectionType;

typedef struct{
	/* Parameters which will be filled when we get the request from client */
	SOCKET SockID;
	HttpRequestType ReqType;
	int8 ClientIP[MAXIMUM_IP_SIZE];
	uint32 ClientPort;
	int8 SelfIP[MAXIMUM_IP_SIZE];
	uint32 SelfPort;
	int8 Header[HTTP_HEADER_SIZE];
	int8 ConnectionClose;
	HttpVersion HTTPVersion;
	uint32 RecvContentLength;
	uint32 RecvContentRangeStart;
	uint32 RecvContentRangeEnd;
	int8 StringAfterURL[256];
	int8 AcceptLanguage[16];
	/* Parameters while will be filled and used while sending the respnse header. */
	HttpResponseCode ResponseCode;
	uint32 SendContentLength;
	uint32 SendContentRangeStart;
	uint32 SendContentRangeEnd;
	int8 ContentLanguage[16];
	int8 ContentType[32];
	int8 AcceptRange[32];
	int8 TransferMode[64];
	int8 LastModifiedTime[48];
}HttpInfo;
/***************** HTTP Related Structures END. *************************/
/* cHttpUtils: This class will consist of all static function which will be used to perform HTTP related operations like
   getting the header, sending the response e.t.c by which any other component can call these functions and all the
   complexity of these functions will be hidden inside this class implementatin.
*/
class cHttpUtils
{
public:
	cHttpUtils(){}
	~cHttpUtils(){}

	/* ReceiveHeader - ReceiveHeader is used receive the HTTP header from the socket.

	   Input Parameters:
	   param1: SOCKET aSockId : socket id from which header need to be received.
	   param2: int8 *aHeaderBuf: pointer to the buffer to which the received data can be copied.
	   param3: cuint32 aHeaderBufLen :Size of the buffer provided in the above 2nd argument.

	   Return Value:
	   int32 - Returns -1 on failure and on successful retieval of header it will return the no of bytes received from socket.
	*/
	static int32 ReceiveHeader(IN SOCKET aSockId, OUT int8 *aHeaderBuf, IN uint32 aHeaderBufLen);

	/* SendRespnseHeader - SendRespnseHeader is used send the response header which will be present at the starting of
	   of the response sent on the socket.

	   Input Parameters:
	   param1: HttpInfo& aHttpInfo : HttpInfo structure which contains all the necessary info about the current HTTP Session.

	   Return Value:
	   ReturnStatus - Returns FAILURE or SUCCESS upon operation of the api.
	*/
	static ReturnStatus SendRespnseHeader(IN HttpInfo& aHttpInfo);

	static ConnectionType GetConnectionType(IN int8 *aHeaderBuf);

	static ReturnStatus ComposeGETRequest(IN int8 *aGetString, IN int8 *aHostIP, IN int32 aHostPort, IN int8 *aLanguage, INOUT int8 *aBuf, INOUT int32 aBufLen);

	static int32 ExtractContentLength(IN int8 *aRespStr);
private:
};

#endif /* HTTP_UTILS_H_INCLUDED */

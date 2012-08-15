#include "http_listener.h"

cHTTPListener::cHTTPListener()
{
	os_memset(cvActiveSockIDs,-1,sizeof(cvActiveSockIDs));
	cvSockID = -1;
}

cHTTPListener::~cHTTPListener()
{
	ActionOnObserver(DELETEOBSERVER);
}

ReturnStatus
cHTTPListener::CreateHTTPListenSocket(IN const int8* const aAddr,IN const uint32 aPort)
{
	if(aAddr && aPort){
		os_memcpy(cvOwnIP,aAddr,MAXIMUM_IP_SIZE);
		cvOwnPort = aPort;
		cvSocket.InitializeParams(aAddr,aPort,(cSocketObserver*)this);
		cvSockID = cvSocket.CreateUnBlockingTCPServerSocket();
		if(FAILURE == cvSockID){
			return FAILURE;
		}
		return SUCCESS;
	}
	return FAILURE;
}

ReturnStatus
cHTTPListener::ReceiveAndParseHeader(IN HttpInfo &aHttpInfo)
{
	int8 *headerPtr = NULL, *stringAfterURL = NULL, *strConnection = NULL, *strContentLength = NULL,*RangePtr = NULL;

	/* Reset All the parameters of incomming HttpInfo Struct except SockID, ClientIP, ClientPort, SelfIP, SelfPort.
	   Because these params should be set only once when the connection is established and should not be changed.*/
	aHttpInfo.ReqType = HTTP_NONE;
	os_memset(aHttpInfo.Header,'\0',sizeof(aHttpInfo.Header));
	aHttpInfo.ConnectionClose = HTTP_0_0;
	aHttpInfo.RecvContentLength = 0;
	os_memset(aHttpInfo.StringAfterURL,'\0',sizeof(aHttpInfo.StringAfterURL));
	aHttpInfo.ResponseCode = HTTP_RESPONSE_NONE;
	aHttpInfo.SendContentLength = 0;
	os_memset(aHttpInfo.ContentLanguage,'\0',sizeof(aHttpInfo.ContentLanguage));
	os_memset(aHttpInfo.ContentType,'\0',sizeof(aHttpInfo.ContentType));

	/* Before calling Receive Header, set recv timeout on the socket. So that we don't wait on the socket for long time. */
	SockOptValue sockOptVal;
	os_memset(&sockOptVal,0,sizeof(sockOptVal));
	sockOptVal.iValue = 60 * 1000; /* Setting the timeout as 60 seconds. */
	os_set_SocketOptions(aHttpInfo.SockID,RECEIVE_TIMEOUT,&sockOptVal);

	/* Now Receive the Header for further operations */
	if(FAILURE == cHttpUtils::ReceiveHeader(aHttpInfo.SockID,aHttpInfo.Header,sizeof(aHttpInfo.Header))){
		return FAILURE;
	}
	headerPtr = aHttpInfo.Header;
	/* Extract the HTTP Request type and fill it inside HttpInfo Struct. */
	if(0 == os_strncmp(headerPtr,"GET",3)){
		aHttpInfo.ReqType = HTTP_GET;
	}else if(0 == os_strncmp(headerPtr,"HEAD",4)){
		aHttpInfo.ReqType = HTTP_HEAD;
	}else if(0 == os_strncmp(headerPtr,"POST",4)){
		aHttpInfo.ReqType = HTTP_POST;
	}
	/* Extract HTTP Version and the String after URL. Ideally the string will present after "GET/POST/HEAD " */
	while(' ' != *headerPtr && '\0' != *headerPtr){ // After this while loop we will reach the position after GET/POST/HEAD string.
		headerPtr++;
	}
	headerPtr++; // Move forward of the space character.
	stringAfterURL = aHttpInfo.StringAfterURL;
	while(' ' != *headerPtr && '\0' != *headerPtr){ // Here inside this while loop we should copy the String after url.
		*stringAfterURL++ = *headerPtr++;
	}
	/* Check whether the string copied above has IP address of host inside it or not. If it has then remove it.
	  No use of the IP. We only need the string after URL.*/
	if(0 == os_strncmp(aHttpInfo.StringAfterURL,"http://",7)){
		int8 extractedString[256] = {'\0'};
		stringAfterURL = &aHttpInfo.StringAfterURL[8];
		os_strncpy(extractedString,os_strchr(stringAfterURL,'/'),sizeof(extractedString));
		os_memset(aHttpInfo.StringAfterURL,'\0',sizeof(aHttpInfo.StringAfterURL));
		os_strncpy(aHttpInfo.StringAfterURL,extractedString,sizeof(aHttpInfo.StringAfterURL));
	}
	headerPtr++; // Move forward of the space character.
	/* Check for the HTTP Version here and fill in inside HttpInfo. */
	if(0 == os_strncmp(headerPtr,"HTTP/1.1",8)){
		aHttpInfo.HTTPVersion = HTTP_1_1;
	}else{
		aHttpInfo.HTTPVersion = HTTP_1_0; // By default set it to HTTP Version 1.0
	}
	/* Set the ConnectionClose parameter of HttpInfo structure as per the data present inside header part. */
	/* By default set the connectionclose parameter as per the HTTP Version. And after that update is as per the data
	  present inside Header part. */
	if(HTTP_1_1 == aHttpInfo.HTTPVersion){
		aHttpInfo.ConnectionClose = 0; // by default don't close the connection for HTTP 1.1. version
	}else if(HTTP_1_0 == aHttpInfo.HTTPVersion){
		aHttpInfo.ConnectionClose = 1; // by default close the connection for HTTP 1.0. version
	}
	/* Find out the connection type from the incoming header. */
	strConnection = os_strcasestr(headerPtr,"\nConnection:");
	if(strConnection){
		while(':' != *strConnection){ // After this while loop we will reach to the end of "Connection:" string value.
			strConnection++;
		}
		strConnection++;
		if(' ' == *strConnection){
			strConnection++;
		}
		if(0 == os_strncasecmp(strConnection,"close",5)){
			aHttpInfo.ConnectionClose = 1;
		}else if(0 == os_strncasecmp(strConnection,"Keep-Alive",10)){
			aHttpInfo.ConnectionClose = 0;
		}
	}

	/* Extract the content length form the incomming header if it exists. */
	strContentLength = os_strcasestr(headerPtr,"\ncontent-length");
	if(strContentLength){
		int8 *strContentLengthEnd = NULL, contentLengthString[32] = {'\0'}, *tempPtr = NULL;
		while(':' != *strContentLength){ // After this while loop we will reach to the end of "content-length:" string value.
			strContentLength++;
		}
		strContentLength++; /* move forward to cross ':' character. */
		/* Remove white spaces if exists any. */
		if(' ' == *strContentLength){
			strContentLength++;
		}
		/* Now get the end of the content length value string. */
		strContentLengthEnd = os_strcasestr(strContentLength,"\r\n");
		if(NULL != strContentLengthEnd){
			tempPtr = contentLengthString;
			while(strContentLength != strContentLengthEnd){
				*tempPtr++ = *strContentLength++;
			}
			/* no we got the string. Just convert this to integer ans store it in HttpInfo Structure. */
			aHttpInfo.RecvContentLength = os_atoi(contentLengthString);
		}
	}

	/* Extract the Range Header form the incomming header if it exists. */
	RangePtr = os_strcasestr(headerPtr,"\nRange");
	if(RangePtr){
		int8 *strRangeEnd = NULL, rangeString[32] = {'\0'}, *strRangeStart = NULL, *tempPtr = NULL;
		strRangeStart = os_strcasestr(RangePtr,"bytes");
		if(NULL != strRangeStart){
			while('=' != *strRangeStart){ // After this while loop we will reach to the end of "content-length:" string value.
				strRangeStart++;
			}
			strRangeStart++; /* move forward to cross '=' character. */
			/* Remove white spaces if exists any. */
			if(' ' == *strRangeStart){
				strRangeStart++;
			}
			/* Now extract the Starting Range. */
			strRangeEnd = os_strchr(strRangeStart,'-');
			if(NULL != strRangeEnd){
				tempPtr = rangeString;
				while(strRangeStart != strRangeEnd){
					*tempPtr++ = *strRangeStart++;
				}
				/* no we got the string. Just convert this to integer ans store it in HttpInfo Structure. */
				aHttpInfo.RecvContentRangeStart = os_atoi(rangeString);

				/* Until Now we extracted the Range Start Value. Now extract the Range End Value. */
				strRangeStart = strRangeEnd;
				os_memset(rangeString,'\0',sizeof(rangeString));

				/* Remove white spaces if exists any. */
				if(' ' == *strRangeStart){
					strRangeStart++;
				}
				strRangeEnd = os_strstr(strRangeStart,"\r\n");
				if(NULL != strRangeEnd){
					tempPtr = rangeString;
					while(strRangeStart != strRangeEnd){
						*tempPtr++ = *strRangeStart++;
					}
					if(os_strlen(rangeString)){
						/* no we got the string. Just convert this to integer ans store it in HttpInfo Structure. */
						aHttpInfo.RecvContentRangeEnd = os_atoi(rangeString);
					}
				}
			}else{
				/*WARNING Inorder to be more roubust we can handle the Range string inside this else condition. But as of now Not needed.
				  In future if you think of adding it don't hegitate to add it. :) */
			}
		}else{
			/* WARNING Inside this else condition we need add the logic to handle the time based Range requests. Currntly I am not adding it.
			   In future if you need it feel free to add it inside this else condition. */
		}
	}
	return SUCCESS;
}

void
cHTTPListener::HandleSocketActivity(IN SocketObserverParams &aSockObsParms)
{
	if(TCP_CLIENT_CONNECTED == aSockObsParms.SockActivity && NULL != aSockObsParms.TCPSockInfoPtr){
		uint32 count = 0, tempCount = 0;
		HttpInfo httpInfo;
		ObserverData *observerDataPtr = NULL;

		/* Reset the HttpInfo structure to eliminate un necessary garbage which can create nucense further. :)*/
		os_memset(&httpInfo,0,sizeof(httpInfo));
		/* Copy the self IP & Port no to the HttpInfo Struct */
		os_memcpy(httpInfo.SelfIP,cvOwnIP,MAXIMUM_IP_SIZE);
		httpInfo.SelfPort = cvOwnPort;
		/* Copy the client's IP & Port no to the HttpInfo Struct */
		os_memcpy(httpInfo.ClientIP,aSockObsParms.TCPSockInfoPtr->PeerIPPtr,MAXIMUM_IP_SIZE);
		httpInfo.ClientPort = aSockObsParms.TCPSockInfoPtr->PeerPort;
		/* Set the Socket ID of this connection. */
		httpInfo.SockID = aSockObsParms.TCPSockInfoPtr->SockID;

		/* First Receive and Parse Header. After that we can notify obeserver based on the connection. */
		if(SUCCESS != ReceiveAndParseHeader(httpInfo)){
			goto HandleSocketActivity_Exit;
		}
		/* By this point, we are done with parsing of the header and filling HttpInfo structure. Now the remaining task
		  is to call the corresponding observer which has registered with the string which matches with the one present
		  inside header received from client.*/
		if(SUCCESS != ActionOnObserver(GETCOUNT,&count)){
			goto HandleSocketActivity_Exit;
		}
		for(tempCount=1; tempCount<=count; tempCount++){
			observerDataPtr = NULL;
			if(SUCCESS != ActionOnObserver(GETOBSERVER,&tempCount,NULL,(void**)&observerDataPtr)){
				goto HandleSocketActivity_Exit;
			}
			if(observerDataPtr->Data.RequestType == httpInfo.ReqType){ // Checking whether Request matches or not.
				/* Checking for the string registerd by Observer with the one came inside header. */
				if(0 == os_strncmp(observerDataPtr->Data.MatchingString,httpInfo.StringAfterURL,os_strlen(observerDataPtr->Data.MatchingString))){
					if(observerDataPtr->Observer){
						/* Before calling the Observers registered function, make sure keep entry of this SockID in
						   cvActiveSockIDs Array. This is needed because in case of shutdown of servers it's better
						   to close all the socket before exiting. For that purpose we are keeping track of the socket
						   ids. */
						for(int32 sockCount=0; sockCount<HTTP_LISTEN_MAX_SOCKET_ID_COUNT; sockCount++){
							if(-1 == cvActiveSockIDs[sockCount]){
								/* We got the empty entry in the array. So update at this location and break. */
								cvActiveSockIDs[sockCount] = httpInfo.SockID;
								break;
							}
						}
						/* If everything is matched then just call the observers function. */
						observerDataPtr->Observer->HeaderReceived(httpInfo);
#if 0
						/* After completing the processing of the request close the socket based on the
						  ConnectionClose flag in HttpInfo Structure. */
						if(1 == httpInfo.ConnectionClose){
							if(0 < httpInfo.SockID){
								cSocket::CloseSocket(httpInfo.SockID);
							}
						}
						/* Instead of keeping the socket open after competing the processing the request completely,
						   it's better to close the socket. In case if the observer want to use the socket for multiple
						   requests he is free to you. But as long as application uses this socket control should not
						   reach here. Control reached here means observer is done with this socket. So we can close
						   it. */
						   cSocket::CloseSocket(httpInfo.SockID);
#endif
					}
				}
			}
		}
HandleSocketActivity_Exit:
		/* Reached here means wheter there is no observer registerd or registered observer is done with the request.
		   in any case we are free to close the socket. But Don't forget to close the Socket irrespective of the
		   HTTP connection(1.0 or 1.1). Otherwise you will loose the socket. */

		/* Before Closing the socket make sure to remove the corresponding entry from the cvActiveSockIDs array. Otherwise
		   Array might overflow and we may close the already closed sockets which is wrong. */
		for(int32 sockCount=0; sockCount<HTTP_LISTEN_MAX_SOCKET_ID_COUNT; sockCount++){
			if(httpInfo.SockID == cvActiveSockIDs[sockCount]){
				/* We got the empty entry in the array. So update at this location and break. */
				cvActiveSockIDs[sockCount] = -1;
				break;
			}
		}

		cSocket::CloseSocket(httpInfo.SockID);
		httpInfo.SockID = -1;

	}else if(UDP_MESSAGE_RECEIVED == aSockObsParms.SockActivity && NULL != aSockObsParms.UDPSockInfoPtr){
		LOGD("cHTTPListener::HandleSocketActivity Error we should not receive UDP message here. Look in to it.");
	}
}

void cHTTPListener::CloseAllConnections()
{
	/* Fist stop the TCP socket server and close the server socket. So that we don't receive any callback form socket layer on the server socket. */
	if(0 > cvSockID){
		cvSocket.StopSocketServer();
		cSocket::CloseSocket(cvSockID);
	}
	/* Go through the cvActiveSockIDs array and close all the active connections. */
	for(int32 sockCount=0; sockCount<HTTP_LISTEN_MAX_SOCKET_ID_COUNT; sockCount++){
		if(-1 != cvActiveSockIDs[sockCount]){
			/* We got the active socket. Close it. */
			cSocket::CloseSocket(cvActiveSockIDs[sockCount]);
		}
	}
}

void cHTTPListener::RegisterObserver(IN HttpRequestType aHttpRequestType, IN const int8 *aMatchingString,
				     IN cHTTPListenerInterface *aObserver)
{
	if(aObserver){
		ObserverData *obsData = (ObserverData*)os_malloc(sizeof(ObserverData));
		os_memcpy(obsData->Data.MatchingString,aMatchingString,sizeof(obsData->Data.MatchingString));
		obsData->Data.RequestType = aHttpRequestType;
		obsData->Observer = aObserver;
		ActionOnObserver(SETOBSERVER,NULL,(void*)obsData);
	}
}

ReturnStatus cHTTPListener::ActionOnObserver(IN ObserverAction aActionOnObserver,INOUT uint32 *aCount,INOUT void *aInputPtr, OUT void **aOutPtr)
{
	static cLinkedList<ObserverData*> ObserverList;

	switch(aActionOnObserver){
		case GETCOUNT:
			if(NULL != aCount){
				*aCount = ObserverList.GetNumberOfElements();
				//LOG_CONSOLE("cHTTPListener::ActionOnObserver GETCOUNT [%d]",*aCount);
			}
			break;
		case GETOBSERVER:
				if(NULL != aCount && NULL != aOutPtr){
					ObserverData *tmpPtr = NULL;
					int32 index = (int32)*aCount;
					ObserverList.GetElementAtIndex(tmpPtr,index);
					*aOutPtr = tmpPtr;
					//LOG_CONSOLE("cHTTPListener::ActionOnObserver GETOBSERVER [%x] Index[%d]",tmpPtr,index);
				}
			break;
		case SETOBSERVER:
			if(aInputPtr){
				ObserverData *tmpPtr = (ObserverData*)aInputPtr;
				ObserverList.Add(tmpPtr);
				//LOG_CONSOLE("cHTTPListener::ActionOnObserver SETOBSERVER [%x]",tmpPtr);
			}
			break;
		case DELETEOBSERVER:
			while(ObserverList.GetNumberOfElements()){
				ObserverData *observerDataPtr = NULL;
				ObserverList.RemoveAtStart(observerDataPtr);
				//LOG_CONSOLE("cHTTPListener::ActionOnObserver DELETEOBSERVER [%x]",observerDataPtr);
				if(observerDataPtr){
					os_free(observerDataPtr);
				}
			}
			break;
		default:
			break;
	}
	return SUCCESS;
}

#include "http_utils.h"

int32 cHttpUtils::ReceiveHeader(IN SOCKET aSockId, OUT int8 *aHeaderBuf, IN uint32 aHeaderBufLen)
{
	uint32 recvCount = 0, httpChunkEnd = 0, headerCompleted = 0;
	int32 recvBytes = 0;
	os_memset((void*)aHeaderBuf,'\0',aHeaderBufLen);
	/* Start Receiving the data form socket one byte at a time. */
	while(recvCount < aHeaderBufLen){
		recvBytes = cSocket::Receive(aSockId, &aHeaderBuf[recvCount],1);
		/* If there is any error while reading the data from socket, then error out. No way to recover it. */
		if(SOCKET_ERROR == recvBytes || 0 >= recvBytes){
			LOGE("cHttpUtils::ReceiveHeader Error recvBytes < 0 || recvBytes == SOCKET_ERROR");
			return FAILURE;
		}
		/* Discard the starting data until we get proper HTTP Method. I mean initial byte received from socket
		   should be the alphabet (May be GET, POST, or HEAD e.t.c). Until we get alphabet discard the initial
		   data assuming it as garbage.
		*/
		if(aHeaderBuf[0] < 0x41){
			continue;
		}
		/* Check for end of the header. In case of HTTP 1.1 we do receive "\r\n\r\n" at the end of header. So
		   if we receive this string then we can assume that we received the complete header. */
		if('\r' == aHeaderBuf[recvCount] && (0 == httpChunkEnd || 2 == httpChunkEnd)){
			httpChunkEnd++;
		}
		else if('\n' == aHeaderBuf[recvCount] && (1 == httpChunkEnd || 3 == httpChunkEnd)){
			httpChunkEnd++;
			if(4 == httpChunkEnd){
				headerCompleted = 1;
				break;
			}
		}else{
			httpChunkEnd = 0;
		}
		recvCount++;
	}
	/* Check whether we received the complete header or not. If we didn't received complete header because of
	   small input buffer then return the partial header and simply read remaining header from socket and discard it.
	*/
	if(0 == headerCompleted){
		int8 tempByte = 0;
		aHeaderBuf[aHeaderBufLen-1] = '\0';
		while(1){
			recvBytes = cSocket::Receive(aSockId, &tempByte,1);
			/* If there is any error while reading the data from socket, then error out. No way to recover it. */
			if(SOCKET_ERROR == recvBytes){
				LOGE("cHttpUtils::ReceiveHeader Error recvBytes == SOCKET_ERROR");
				return FAILURE;
			}
			/* Check for end of the header. In case of HTTP 1.1 we do receive "\r\n\r\n" at the end of header. So
			  if we receive this string then we can assume that we received the complete header. */
			if('\r' == aHeaderBuf[recvCount] && (0 == httpChunkEnd || 2 == httpChunkEnd)){
				httpChunkEnd++;
			}
			else if('\n' == aHeaderBuf[recvCount] && (1 == httpChunkEnd || 3 == httpChunkEnd)){
				httpChunkEnd++;
				if(4 == httpChunkEnd){
					headerCompleted = 1;
					break;
				}
			}else{
				httpChunkEnd = 0;
			}
		}
	}
	return recvCount;
}

ConnectionType cHttpUtils::GetConnectionType(IN int8 *aHeaderBuf)
{
	int8 *tempPtr = os_strcasestr(aHeaderBuf,"Connection");
	if(tempPtr == NULL){
		return CONNECTION_CLOSE;
	}
	int32 length = 0;
	tempPtr += os_strlen((int8*)"Connection");
	length = os_strlen(tempPtr);
	while((' ' == *tempPtr || ':' == *tempPtr) && length > 0){
		tempPtr++;
		length--;
	}
	if(os_strncmp(tempPtr,"close",5) == 0){
		return CONNECTION_CLOSE;
	}else{
		return CONNECTION_KEEP_ALIVE;
	}
}

ReturnStatus cHttpUtils::SendRespnseHeader(IN HttpInfo& aHttpInfo)
{
	int8 responseHeader[HTTP_HEADER_SIZE] = {'\0'};
	int32 responseLen = 0, responseBufSize = HTTP_HEADER_SIZE,len = 0;
	int8 *responseBuf = responseHeader, *tempBuf = NULL, *tempBuf1 = NULL;

	responseLen = responseBufSize;

	os_snprintf(responseBuf+(responseBufSize-responseLen),responseLen,"HTTP/1.1 ");
	responseLen = responseBufSize - os_strlen(responseBuf);

	/* Get the response code string as per the response code no. */
	if(HTTP_RESPONSE_OK == aHttpInfo.ResponseCode){
		os_snprintf(responseBuf+(responseBufSize-responseLen),responseLen,"200 OK\r\n");
		responseLen = responseBufSize - os_strlen(responseBuf);
	}else if(HTTP_RESPONSE_BAD_REQUEST == aHttpInfo.ResponseCode){
		os_snprintf(responseBuf+(responseBufSize-responseLen),responseLen,"400 Bad Request\r\n");
		responseLen = responseBufSize -  os_strlen(responseBuf);
	}else if(HTTP_RESPONSE_NOT_FOUND == aHttpInfo.ResponseCode){
		os_snprintf(responseBuf+(responseBufSize-responseLen),responseLen,"404 Not Found\r\n");
		responseLen = responseBufSize -  os_strlen(responseBuf);
	}

	if(os_strlen(aHttpInfo.ContentType)){
		if(0 == os_strncmp(aHttpInfo.ContentType,"text/xml",8)){
			os_snprintf(responseBuf+(responseBufSize-responseLen),responseLen,"Content-Type: %s; charset=\"utf-8\"\r\n",aHttpInfo.ContentType);
			responseLen = responseBufSize -  os_strlen(responseBuf);
		}else{
			os_snprintf(responseBuf+(responseBufSize-responseLen),responseLen,"Content-Type: %s\r\n",aHttpInfo.ContentType);
			responseLen = responseBufSize -  os_strlen(responseBuf);
		}
	}

	if(os_strlen(aHttpInfo.ContentLanguage)){
		os_snprintf(responseBuf+(responseBufSize-responseLen),responseLen,"CONTENT-LANGUAGE: %s\r\n",aHttpInfo.ContentLanguage);
		responseLen = responseBufSize -  os_strlen(responseBuf);
	}

	tempBuf = os_get_date();
	if(NULL != tempBuf){
		len = os_strlen(tempBuf)+5;
		tempBuf1 = (int8*)os_malloc(len);
		if(NULL != tempBuf1){
			os_memset(tempBuf1,0,len);
			os_strncpy(tempBuf1,tempBuf,len);
			tempBuf1[os_strlen(tempBuf1)-1] = '\0';
			os_snprintf(responseBuf+(responseBufSize-responseLen),responseLen,"Date: %s\r\n",tempBuf1);
			responseLen = responseBufSize -  os_strlen(responseBuf);

			os_free(tempBuf1);
		}
	}

	os_snprintf(responseBuf+(responseBufSize-responseLen),responseLen,"Content-Length: %d\r\n",aHttpInfo.SendContentLength);
	responseLen = responseBufSize -  os_strlen(responseBuf);

	os_snprintf(responseBuf+(responseBufSize-responseLen),responseLen,"Accept-Ranges: bytes\r\n");
	responseLen = responseBufSize -  os_strlen(responseBuf);

	if(os_strstr(aHttpInfo.Header,"Range")){
		os_snprintf(responseBuf+(responseBufSize-responseLen),responseLen,"Content-Range: bytes %d-%d/%d\r\n",
										   aHttpInfo.SendContentRangeStart,
										   aHttpInfo.SendContentLength,
										   aHttpInfo.SendContentLength);
		responseLen = responseBufSize -  os_strlen(responseBuf);
	}

	if(os_strlen(aHttpInfo.TransferMode)){
		os_snprintf(responseBuf+(responseBufSize-responseLen),responseLen,"transferMode.dlna.org: %s\r\n",aHttpInfo.TransferMode);
		responseLen = responseBufSize -  os_strlen(responseBuf);
	}

	if(os_strlen(aHttpInfo.LastModifiedTime)){
		os_snprintf(responseBuf+(responseBufSize-responseLen),responseLen,"Last-Modified: %s\r\n",aHttpInfo.LastModifiedTime);
		responseLen = responseBufSize -  os_strlen(responseBuf);
	}

	os_snprintf(responseBuf+(responseBufSize-responseLen),responseLen,"Connection: %s\r\n",(aHttpInfo.ConnectionClose)?"close":"Keep-Alive");
	responseLen = responseBufSize -  os_strlen(responseBuf);

	os_snprintf(responseBuf+(responseBufSize-responseLen),responseLen,"EXT:\r\nSERVER: %s/%d.%d, UPnP/1.0, %s/%d.%d\r\n\r\n",
									    cUPnPBase::GetSSDPInfo()->SSDPOSName,
									    cUPnPBase::GetSSDPInfo()->SSDPOSMajorVersionNo,
									    cUPnPBase::GetSSDPInfo()->SSDPOSMinorVersionNo,
									    cUPnPBase::GetSSDPInfo()->SSDPProductName,
									    cUPnPBase::GetSSDPInfo()->SSDPProductMajorVersionNo,
									    cUPnPBase::GetSSDPInfo()->SSDPProductMinorVersionNo);
	responseLen = responseBufSize -  os_strlen(responseBuf);

	if(os_strlen(responseHeader) != cSocket::Send(aHttpInfo.SockID,responseHeader,os_strlen(responseHeader))){
		LOGE("cUPnPDescription::SendRespnseHeader Failed to send the response header on socket[%d]",aHttpInfo.SockID);
		return FAILURE;
	}
	return SUCCESS;
}

ReturnStatus
cHttpUtils::ComposeGETRequest(IN int8 *aGetString, IN int8 *aHostIP, IN int32 aHostPort, IN int8 *aLanguage, INOUT int8 *aBuf, INOUT int32 aBufLen)
{
	int32 len = aBufLen;

	if(NULL == aGetString){
		return FAILURE;
	}
	/* First clear all the memory holded by aBuf */
	os_memset(aBuf,0,aBufLen);

	/* Start composing GET request */
	os_snprintf(aBuf+(aBufLen-len),len,"GET %s HTTP/1.1\r\n",aGetString);
	len = aBufLen - os_strlen(aBuf);

	if(aHostIP){
		/* If HostIP is given then we need to include the HOST header.. So lets do that.. */
		os_snprintf(aBuf+(aBufLen-len),len,"HOST: %s:%d\r\n",aHostIP,aHostPort);
		len = aBufLen - os_strlen(aBuf);
	}
	if(aLanguage){
		/* If aLanguage is given then we need to include ACCEPT-LANGUAGE header.. So lets do that.. */
		os_snprintf(aBuf+(aBufLen-len),len,"ACCEPT-LANGUAGE: %s\r\n",aLanguage);
		len = aBufLen - os_strlen(aBuf);
	}
	/* Finally add \r\n at the end of the request.. */
	os_snprintf(aBuf+(aBufLen-len),len,"\r\n");
	len = aBufLen - os_strlen(aBuf);

	return SUCCESS;
}

int32
cHttpUtils::ExtractContentLength(IN int8 *aRespStr)
{
	int8 *tempPtr = NULL, *tempPtr1 = NULL, contentLengthStr[16] = {'\0'};

	tempPtr = os_strcasestr(aRespStr,"CONTENT-LENGTH");
	if(NULL == tempPtr){
		return FAILURE;
	}
	tempPtr += 14;
	/* going to point the start of the content length string.. */
	while(' ' == *tempPtr || ':' == *tempPtr){
		tempPtr++;
	}

	tempPtr1 = contentLengthStr;
	while(' ' != *tempPtr && '\0' != *tempPtr && '\r' != *tempPtr && '\n' != *tempPtr){
		*tempPtr1++ = *tempPtr++;
	}

	if(os_strlen(contentLengthStr)){
		return os_atoi(contentLengthStr);
	}else{
		return FAILURE;
	}
}

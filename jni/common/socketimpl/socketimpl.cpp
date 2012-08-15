#include "socketimpl.h"
/* Global mutex variable which can be used only inside this file. */
//os_mutex gMutex = OS_MUTEX_INITIALIZER;

/* Declare the static variable used inside the class. This needs to be done to avid the linking errors. Something strange. Don't worry. You get used to it :) */
bool cSocket::cvCloseAllSessions = false;
int32 cSocket::cvConnectedSockIDs[100] = {-1};
int32 cSocket::cvNoOfConnectedSockIDs = 0;

cSocket::cSocket()
{
	/* Initialize the class variables at the beginning. */
	os_memset(cvAddr,'\0',sizeof(cvAddr));
	os_memset(cvMembershipAddr,'\0',sizeof(cvMembershipAddr));
	cvPort = 0;
	cvObserver = NULL;
	cvState = IDLE;
	cvSockID = -1;
	cvClientSockID = -1;
	cvListenCount = 10; // By default listen count is set to be 10. Can be updated using SetListenCount api.
	cvRecvBufSize = 1024; // By default recv buffer size is set to 1024. Can be updated using SetRecvBufferSize api.
	cvSocketServerActive = false;
	cSocket::cvCloseAllSessions = false;
}

cSocket::cSocket(IN const int8* const aAddr,IN const uint32 aPort,IN const cSocketObserver * aObserver)
{
	/* Initialize the class variables at the beginning. */
	os_memset(cvAddr,'\0',sizeof(cvAddr));
	os_memset(cvMembershipAddr,'\0',sizeof(cvMembershipAddr));
	cvPort = 0;
	cvObserver = NULL;
	cvState = IDLE;
	cvSockID = -1;
	cvClientSockID = -1;
	cvListenCount = 10; // By default listen count is set to be 10. Can be updated using SetListenCount api.
	cvRecvBufSize = 1024; // By default recv buffer size is set to 1024. Can be updated using SetRecvBufferSize api.
	cvSocketServerActive = false;
	cSocket::cvCloseAllSessions = false;

	/* Set the incomming parameters accordingly */
	if(aAddr){
		os_strncpy(cvAddr,aAddr,sizeof(cvAddr));
		LOGD("cSocket::cSocket Initializing with Address [%s] Port[%d]",cvAddr,cvPort);
	}
	cvPort = aPort;
	cvObserver = (cSocketObserver*)aObserver;
}

void cSocket::InitializeParams(IN const int8* const aAddr,IN const uint32 aPort, IN const cSocketObserver * aObserver)
{
	/* Set the incomming parameters accordingly */
	if(aAddr){
		os_strncpy(cvAddr,aAddr,sizeof(cvAddr));
		LOGD("cSocket::InitializeParams Initializing with Address [%s] port[%d]",cvAddr,aPort);
	}
	cvPort = aPort;
	cvObserver = (cSocketObserver*)aObserver;
}

void cSocket::SetListenCount(IN const uint32 aListenCount)
{
	/* Set the Listen count for the clients in case of the server sockets */
	cvListenCount = aListenCount;
}

void cSocket::SetRecvBufferSize(IN const uint32	aRevBufSize)
{
	/* Set the recv buffer size used in recvfrom api to receive the messages in case of the UDP server sockets */
	cvRecvBufSize = aRevBufSize;
}

void cSocket::SetMembershipAddr(IN const int8* const aMemshipAddr)
{
	/* Set the Membership address used in case of udp broadcast sockets to join the particular Addressgroup. */
	if(os_strlen((int8*)aMemshipAddr)){
		os_strncpy(cvMembershipAddr,aMemshipAddr,sizeof(cvMembershipAddr));
		LOGD("cSocket::SetMembershipAddr stored the Membership Address [%s]",cvMembershipAddr);
	}
}

void cSocket::SetFilteringUDPMessageString(IN const int8* const aUDPFilterString)
{
	/* Set the Membership address used in case of udp broadcast sockets to join the particular Addressgroup. */
	if(os_strlen((int8*)aUDPFilterString)){
		UDPFilterString filterString;
		os_memset(&filterString,0,sizeof(filterString));
		os_strncpy(filterString.FilterString,aUDPFilterString,sizeof(filterString.FilterString));
		cvUDPFilterStringList.Add(filterString);
		LOGD("cSocket::SetFilteringUDPMessageString stored UDP Filtering String [%s]",filterString.FilterString);
	}
}

SOCKET cSocket::CreateUnBlockingTCPServerSocket(void)
{
	/* Local Variable section. Just for clean coding, make sure to declare all the local variables used inside this function at the beginning itself rather
	   than declaring them whenever they are necessary. More over this is mandatory if we follow c99 standards. */
	int32 iRetVal = 0;
	SOCK_INET_ADDR_STRUCT sSockAddr;
	cvSocketServerActive = true;


	os_memset(&sSockAddr,'\0',sizeof(sSockAddr));
	/* Creating the TCP Socket. */
	cvSockID = socket(AF_INET,SOCK_STREAM,0);
	if(cvSockID < 0){
		LOGE("cSocket::CreateUnBlockingTCPServerSocket Failed to create the socket. retval [%d]",cvSockID);
		return FAILURE;
	}
	/* Assaigning the Name to the above created Socket */
	sSockAddr.sin_family = AF_INET;
	sSockAddr.sin_port = htons(cvPort);
	sSockAddr.sin_addr.s_addr = inet_addr((int8*)cvAddr);
	iRetVal = bind(cvSockID,(SOCK_ADDR_STRUCT *)&sSockAddr,(socklen_t)sizeof(sSockAddr));
	if(-1 == iRetVal){
		LOGE("cSocket::CreateUnBlockingTCPServerSocket Failed to bind the socket. retval [%d]",iRetVal);
		CloseSocket(cvSockID);
		return FAILURE;
	}
	/* Creating the listen queue for listening the clients. */
	iRetVal = listen(cvSockID,cvListenCount);
	if(-1 == iRetVal){
		LOGE("cSocket::CreateUnBlockingTCPServerSocket Failed to listen on the socket. retval [%d]",iRetVal);
		CloseSocket(cvSockID);
		return FAILURE;
	}
	/* before going to the bocking section of the code(Looking for clients in a continuous while loop, update the socket id into the static
	   cvConnectedSockIDs array. This is useful when the system about to go down. When system is about to down, then the application can call
	   the static function CloseAllSessions and inside that function we close all the open sockets such that no unclosed connections are left back.*/
	  cSocket::cvConnectedSockIDs[cSocket::cvNoOfConnectedSockIDs++] = cvSockID;

	cvLocalCallbackStruct *localCallbackStruct = (cvLocalCallbackStruct*)os_malloc(sizeof(cvLocalCallbackStruct));
	  if(NULL == localCallbackStruct){
		LOGE("cSocket::CreateUnBlockingTCPServerSocket Failed to Allocate memory for cvLocalCallbackStruct");
		CloseSocket(cvSockID);
		return FAILURE;
	  }
	  /* Initialize all the parameters to the sturcture so that the same params can be used inside the callback function. */
	  os_memset(localCallbackStruct,'\0',sizeof(cvLocalCallbackStruct));
	  localCallbackStruct->Observer = cvObserver;
	  localCallbackStruct->SockID = cvSockID;
	  localCallbackStruct->ServerActive = &cvSocketServerActive;
	  os_strncpy(localCallbackStruct->SelfAddr,cvAddr,sizeof(localCallbackStruct->SelfAddr));
	  localCallbackStruct->SelfPort = cvPort;

	   /* Create the new thread to wait for the clinets. So that the main thread can be returned to the observer.. */
	  if(-1 == Create_New_Thread(cSocket::LookForClients,(void*)localCallbackStruct,THREAD_PRIORITY_IS_NORMAL)){
		LOGE("cSocket::CreateUnBlockingTCPServerSocket Thread Creation Failed");
		os_free(localCallbackStruct);
		close(cvSockID);
		return FAILURE;
	  }
	  return cvSockID;
}

void* cSocket::LookForClients(void *avpIn)
{
	int32 *bServerActive = NULL;
	int32 iSockID = 0;
	cSocketObserver *observer = NULL;
	int8 selfAddr[MAXIMUM_IP_SIZE] = {'\0'};
	uint32 selfPort = 0;
	if(avpIn){
		cvLocalCallbackStruct *localCallbackStruct = (cvLocalCallbackStruct*)avpIn;
		bServerActive = localCallbackStruct->ServerActive;
		iSockID = localCallbackStruct->SockID;
		observer = localCallbackStruct->Observer;
		os_strncpy(selfAddr,localCallbackStruct->SelfAddr,sizeof(selfAddr));
		selfPort = localCallbackStruct->SelfPort;
		os_free(avpIn);
	}else{
		return NULL;
	}
	/* Waiting on the socket for clients to connect in the continuous while loop. */
	while(*bServerActive && !cSocket::cvCloseAllSessions){
	      SOCK_INET_ADDR_STRUCT sClientSockAddr;
	      int32 clientSockID; socklen_t iClientSockAddrLen = sizeof(sClientSockAddr);
	      /* Accepting the clients from above created server socket. */
	      clientSockID = accept(iSockID,(SOCK_ADDR_STRUCT*)&sClientSockAddr,&iClientSockAddrLen);
	      /* If we got error. Then something is wrong. Keep on trying. */
	      if(-1 == clientSockID){
			continue;
	      }
		/* Create the sturucture which holds the observer pointer and the client socket ID. The memory for this
		  structure should be allocated dynamically and pass this to Create_New_Thread function such that the
		  same pointer will be passed to the thread registered function and inside that function the memory for
		  of this structure will be deallocated.*/
		cvThreadStruct *threadStruct = (cvThreadStruct*)os_malloc(sizeof(cvThreadStruct));
		if(NULL == threadStruct){
			close(clientSockID);
			continue;
		}
		/* Initialize all the parameters to the sturcture so that the same params can be used inside the callback function. */
		os_memset(threadStruct,'\0',sizeof(cvThreadStruct));
		threadStruct->Observer = observer;
		threadStruct->SockID = clientSockID;
		threadStruct->SockActivity = TCP_CLIENT_CONNECTED;
		os_strncpy(threadStruct->SelfAddr,selfAddr,sizeof(threadStruct->SelfAddr));
		threadStruct->SelfPort = selfPort;
		if(iClientSockAddrLen){
			int8 *cpTemp = inet_ntoa(sClientSockAddr.sin_addr);
			os_strncpy(threadStruct->ClientAddr,cpTemp,sizeof(threadStruct->ClientAddr));
			threadStruct->ClientPort = ntohs(sClientSockAddr.sin_port);
		}

	      /* Create the new thread for each connected clients. */
	      if(-1 == Create_New_Thread(cSocket::AcceptCallback,(void*)threadStruct,THREAD_PRIORITY_IS_NORMAL)){
			os_free(threadStruct);
			close(clientSockID);
			continue;
		}
	}
	Terminate_Thread();
	return NULL;
}

SOCKET cSocket::CreateBlockingTCPServerSocket(void)
{
	/* Local Variable section. Just for clean coding, make sure to declare all the local variables used inside this function at the beginning itself rather
	   than declaring them whenever they are necessary. More over this is mandatory if we follow c99 standards. */
	int32 iRetVal = 0;
	SOCK_INET_ADDR_STRUCT sSockAddr;
	cvSocketServerActive = true;


	os_memset(&sSockAddr,'\0',sizeof(sSockAddr));
	/* Creating the TCP Socket. */
	cvSockID = socket(AF_INET,SOCK_STREAM,0);
	if(cvSockID < 0){
		LOGE("cSocket::CreateBlockingTCPServerSocket Socket creation failed. retval[%d]",iRetVal);
		return FAILURE;
	}
	/* Assaigning the Name to the above created Socket */
	sSockAddr.sin_family = AF_INET;
	sSockAddr.sin_port = htons(cvPort);
	sSockAddr.sin_addr.s_addr = inet_addr((int8*)cvAddr);
	iRetVal = bind(cvSockID,(SOCK_ADDR_STRUCT *)&sSockAddr,(socklen_t)sizeof(sSockAddr));
	if(-1 == iRetVal){
		LOGE("cSocket::CreateBlockingTCPServerSocket Socket bind system call failed. retval[%d]",iRetVal);
		CloseSocket(cvSockID);
		return FAILURE;
	}
	/* Creating the listen queue for listening the clients. */
	iRetVal = listen(cvSockID,cvListenCount);
	if(-1 == iRetVal){
		LOGE("cSocket::CreateBlockingTCPServerSocket Socket listen system call failed. retval[%d]",iRetVal);
		CloseSocket(cvSockID);
		return FAILURE;
	}
	/* before going to the bocking section of the code, update the socket id into the static cvConnectedSockIDs array. This is useful
	  when the system about to go down. When system is about to down, then the application can call the static function CloseAllSessions and
	  inside that function we close all the open sockets such that no unclosed connections are left back.*/
	cSocket::cvConnectedSockIDs[cSocket::cvNoOfConnectedSockIDs++] = cvSockID;

	/* Waiting on the socket for clients to connect in the continuous while loop. */
	while(cvSocketServerActive && !cSocket::cvCloseAllSessions){
		SOCK_INET_ADDR_STRUCT sClientSockAddr;
		int32 clientSockID; socklen_t iClientSockAddrLen = sizeof(sClientSockAddr);
		/* Accepting the clients from above created server socket. */
		clientSockID = accept(cvSockID,(SOCK_ADDR_STRUCT*)&sClientSockAddr,&iClientSockAddrLen);
		/* If we got error. Then something is wrong. Keep on trying. */
		if(-1 == clientSockID){
			LOGE("cSocket::CreateBlockingTCPServerSocket Socket accept system call failed. retval[%d]",clientSockID);
			continue;
		}
	      /* Create the sturucture which holds the observer pointer and the client socket ID. The memory for this
		structure should be allocated dynamically and pass this to Create_New_Thread function such that the
		same pointer will be passed to the thread registered function and inside that function the memory for
		of this structure will be deallocated.*/
	      cvThreadStruct *threadStruct = (cvThreadStruct*)os_malloc(sizeof(cvThreadStruct));
	      if(NULL == threadStruct){
			LOGE("cSocket::CreateBlockingTCPServerSocket memory allocation for cvThreadStruct failed.");
			CloseSocket(clientSockID);
			continue;
	      }
		/* Initialize all the parameters to the sturcture so that the same params can be used inside the callback function. */
		os_memset(threadStruct,'\0',sizeof(cvThreadStruct));
		threadStruct->Observer = cvObserver;
		threadStruct->SockID = clientSockID;
		threadStruct->SockActivity = TCP_CLIENT_CONNECTED;
		os_strncpy(threadStruct->SelfAddr,cvAddr,sizeof(threadStruct->SelfAddr));
		threadStruct->SelfPort = cvPort;
		if(iClientSockAddrLen){
			LOGE("cSocket::CreateBlockingTCPServerSocket Copying the connected clients ip address to threadStruct.");
			int8 *cpTemp = inet_ntoa(sClientSockAddr.sin_addr);
			os_strncpy(threadStruct->ClientAddr,cpTemp,sizeof(threadStruct->ClientAddr));
			threadStruct->ClientPort = ntohs(sClientSockAddr.sin_port);
		}

		/* Create the new thread for each connected clients. */
		if(-1 == Create_New_Thread(cSocket::AcceptCallback,(void*)threadStruct,THREAD_PRIORITY_IS_NORMAL)){
			LOGE("cSocket::CreateBlockingTCPServerSocket Thread Creation Failed");
			os_free(threadStruct);
			CloseSocket(clientSockID);
			continue;
		}
	}
	return SUCCESS;
}

SOCKET cSocket::CreateTCPClientSocket(void)
{
	int32 iRetVal = 0;
	SOCK_INET_ADDR_STRUCT sSockAddr;
	cvSocketServerActive = true;
	os_memset(&sSockAddr,'\0',sizeof(sSockAddr));
	/* Creating the TCP Socket. */
	cvSockID = socket(AF_INET,SOCK_STREAM,0);
	/* Assaigning the Name to the above created Socket */
	sSockAddr.sin_family = AF_INET;
	sSockAddr.sin_port = htons(cvPort);
	sSockAddr.sin_addr.s_addr = inet_addr((int8*)cvAddr);
	iRetVal = connect(cvSockID,(SOCK_ADDR_STRUCT *)&sSockAddr,(socklen_t)sizeof(sSockAddr));
	if(-1 == iRetVal){
		LOGE("cSocket::CreateTCPClientSocket Socket connect system call failed. retval[%d]",iRetVal);
		CloseSocket(cvSockID);
		return FAILURE;
	}
	return cvSockID;
}

SOCKET cSocket::CreateUDPSocket()
{
	int32 iRetVal = 0;
	SOCK_INET_ADDR_STRUCT sSockAddr;
	cvSocketServerActive = true;
	os_memset(&sSockAddr,'\0',sizeof(sSockAddr));
	/* Creating the UDP Socket. */
	cvSockID = socket(AF_INET,SOCK_DGRAM,0);
	if(cvSockID < 0){
		LOGE("cSocket::CreateUDPSocket socket creation failed.");
		return FAILURE;
	}

	/* Set Socket Option for Reusing the Addresses */
	os_set_SocketOptions(cvSockID,REUSE_ADDR);

	/* Assaigning the Name to the above created Socket */
	sSockAddr.sin_family = AF_INET;
	sSockAddr.sin_port = htons(cvPort);
	sSockAddr.sin_addr.s_addr = inet_addr((int8*)cvAddr);
	iRetVal = bind(cvSockID,(SOCK_ADDR_STRUCT *)&sSockAddr,(socklen_t)sizeof(sSockAddr));
	if(-1 == iRetVal){
		LOGE("cSocket::CreateUDPSocket socket bind system call failed. retval[%d]",iRetVal);
		//perror("Reason for UDP Socket Bind:");
		CloseSocket(cvSockID);
		return FAILURE;
	}
	return cvSockID;
}

int32 cSocket::UDPBlockingListenCall()
{
	int32 iRetVal = 0, count = 0;
	cvSocketServerActive = true;
	flag matchingStringFound = false;

	/* If membesrship address is set then join that particular group before listening for messages. */
	if(os_strlen(cvMembershipAddr)){
		SockOptValue sValue1, sValue2;
		sValue1.vpPtr = (void*)cvAddr;
		sValue2.vpPtr = (void*)cvMembershipAddr;
		LOGE("cSocket::UDPBlockingListenCall Setting the socket option for adding the membership \
			   sValue1.vpPtr[%s] sValue2.vpPtr[%s] ",(int8*)sValue1.vpPtr,(int8*)sValue2.vpPtr);
		os_set_SocketOptions(cvSockID,ADD_MEMBERSHIP,&sValue1,&sValue2);
	}
	/* before going to the bocking section of the code, update the socket id into the static cvConnectedSockIDs array. This is useful
	  when the system about to go down. When system is about to down, then the application can call the static function CloseAllSessions and
	  inside that function we close all the open sockets such that no unclosed connections are left back.*/
	cSocket::cvConnectedSockIDs[cSocket::cvNoOfConnectedSockIDs++] = cvSockID;

	/* Start listening for the messages comming on this port. If we receive some messages then intimate to observer by using callback function. */
	while(cvSocketServerActive && !cSocket::cvCloseAllSessions){
	  SOCK_INET_ADDR_STRUCT sClientSockAddr;
	  socklen_t iClientSockAddrLen = sizeof(sClientSockAddr);
	  /* Allocate the memory for recv buffer. Observer is responsible for releasing this memory.Ohterwise Memory leak will happen. Be Careful. */
	  int8* acpRecvBuf = (int8*)os_malloc(cvRecvBufSize);
	  if(NULL == acpRecvBuf){
		LOGE("cSocket::UDPBlockingListenCall memory allocation for cvRecvBufSize Failed.");
		CloseSocket(cvSockID);
		return FAILURE;
	  }

	  /* Start looking for the messages on the above socket. */
	  iRetVal = recvfrom(cvSockID,acpRecvBuf,cvRecvBufSize,0,(SOCK_ADDR_STRUCT*)&sClientSockAddr,&iClientSockAddrLen);
	  /* If we got error. Then something is wrong. Keep on trying. */
	  if(iRetVal <= 0){
		LOGE("cSocket::UDPBlockingListenCall return value of recvfrom system call is <= 0. retval[%d]",iRetVal);
		continue;
	  }
	  matchingStringFound = false;
	  /* Check for the Matching String if cvUDPFilterString is set. And only proceed further it that string is
	     contained inside the UDP message received. If cvUDPFilterString is not set then no problem. Go ahead with
	     out any check.*/
	  for(count=1; count <= cvUDPFilterStringList.GetNumberOfElements(); count++){
		UDPFilterString filterString;
		os_memset(&filterString,'\0',sizeof(filterString));
		cvUDPFilterStringList.GetElementAtIndex(filterString,count);
		if(NULL != os_strcasestr(acpRecvBuf,filterString.FilterString)){
			LOGE("cSocket::UDPBlockingListenCall matching string [%s] is found in the UDP Message received. Hence informing to the observer.",filterString.FilterString);
			matchingStringFound = true;
			break;
		}
	  }

	  /* If none of the filtering strings matched then no need to inform the oberserver... Just ignore.. */
	  if(false == matchingStringFound && (0 < cvUDPFilterStringList.GetNumberOfElements())){
		//LOGD("cSocket::UDPBlockingListenCall none of the filtering strings matched to the received message..Hence not informing to the observer.");
		continue;
	  }
	  LOGD("cSocket::UDPBlockingListenCall Matching string found. Hence intimating to listener");
	  /* Create the sturucture which holds the observer pointer and the client socket ID. The memory for this
	     structure should be allocated dynamically and pass this to Create_New_Thread function such that the
	     same pointer will be passed to the thread registered function and inside that function the memory for
	     of this structure will be deallocated.*/
	  cvThreadStruct *threadStruct = (cvThreadStruct*)os_malloc(sizeof(cvThreadStruct));
	  if(NULL == threadStruct){
		LOGE("cSocket::UDPBlockingListenCall memory allocation for cvThreadStruct failed.");
		CloseSocket(cvSockID);
		return FAILURE;
	  }
	  os_memset(threadStruct,'\0',sizeof(cvThreadStruct));
	  threadStruct->Observer = cvObserver;
	  threadStruct->SockID = cvSockID;
	  threadStruct->SockActivity = UDP_MESSAGE_RECEIVED;
	  if(iClientSockAddrLen){
		//LOGD("cSocket::UDPBlockingListenCall copying the clients ip address to threadStruct.");
		int8 *cpTemp = inet_ntoa(sClientSockAddr.sin_addr);
		os_strncpy(threadStruct->ClientAddr,cpTemp,sizeof(threadStruct->ClientAddr));
	  }
	  threadStruct->RecvBufPtr = acpRecvBuf;
	  threadStruct->RecvBufLen = iRetVal;
	  os_strncpy(threadStruct->SelfAddr,cvAddr,sizeof(threadStruct->SelfAddr));
	  threadStruct->SelfPort = cvPort;
	  /* Create the new thread for each connected clients. */
	  if(-1 == Create_New_Thread(cSocket::AcceptCallback,(void*)threadStruct,THREAD_PRIORITY_IS_NORMAL)){
		LOGE("cSocket::UDPBlockingListenCall Thread Creation Failed");
		os_free(threadStruct);
		continue;
	  }
	}
	return SUCCESS;
}

int32 cSocket::UDPUnBlockingListenCall()
{
	cvSocketServerActive = true;

	/* If membesrship address is set then join that particular group before listening for messages. */
	if(os_strlen(cvMembershipAddr)){
		SockOptValue sValue1, sValue2;
		sValue1.vpPtr = (void*)cvMembershipAddr;
		sValue2.vpPtr = (void*)cvAddr;
		LOGD("cSocket::UDPUnBlockingListenCall Setting the socket option for adding the membership sValue1.vpPtr[%s] sValue2.vpPtr[%s] ",
			    (int8*)sValue1.vpPtr,(int8*)sValue2.vpPtr);
		os_set_SocketOptions(cvSockID,ADD_MEMBERSHIP,&sValue1,&sValue2);
	}

	cvLocalCallbackStruct *localCallbackStruct = (cvLocalCallbackStruct*)os_malloc(sizeof(cvLocalCallbackStruct));
	  if(NULL == localCallbackStruct){
		LOGE("cSocket::UDPUnBlockingListenCall memory allocation for cvLocalCallbackStruct failed.");
		CloseSocket(cvSockID);
		return FAILURE;
	  }
	  /* Initialize all the parameters to the sturcture so that the same params can be used inside the callback function. */
	  os_memset(localCallbackStruct,'\0',sizeof(cvLocalCallbackStruct));
	  localCallbackStruct->Observer = cvObserver;
	  localCallbackStruct->SockID = cvSockID;
	  localCallbackStruct->ServerActive = &cvSocketServerActive;
	  localCallbackStruct->RecvBufSize = cvRecvBufSize;
	  os_strncpy(localCallbackStruct->SelfAddr,cvAddr,sizeof(localCallbackStruct->SelfAddr));
	  localCallbackStruct->SelfPort = cvPort;
	  localCallbackStruct->filterStringsListPtr = &cvUDPFilterStringList;
	   /* Create the new thread to wait for the clinets. So that the main thread can be returned to the observer.. */
	  if(-1 == Create_New_Thread(cSocket::ReceiveBroadcastMessages,(void*)localCallbackStruct,THREAD_PRIORITY_IS_NORMAL)){
		LOGE("cSocket::UDPUnBlockingListenCall Thread Creation Failed");
		os_free(localCallbackStruct);
		close(cvSockID);
		return FAILURE;
	  }
	  return SUCCESS;
}
void* cSocket::ReceiveBroadcastMessages(void *avpIn)
{
	int32 *bServerActive = NULL;
	int32 iSockID = 0;
	int32 iRecvBufSize = 0, count = 0;
	cSocketObserver *observer = NULL;
	int8 selfAddr[MAXIMUM_IP_SIZE] = {'\0'};
	uint32 selfPort = 0;
	cLinkedList<UDPFilterString> *filterStrListPtr = NULL;
	flag matchingStringFound = false;

	if(avpIn){
		cvLocalCallbackStruct *localCallbackStruct = (cvLocalCallbackStruct*)avpIn;
		bServerActive = localCallbackStruct->ServerActive;
		iSockID = localCallbackStruct->SockID;
		observer = localCallbackStruct->Observer;
		iRecvBufSize = localCallbackStruct->RecvBufSize;
		selfPort = localCallbackStruct->SelfPort;
		os_strncpy(selfAddr,localCallbackStruct->SelfAddr,sizeof(selfAddr));
		filterStrListPtr = localCallbackStruct->filterStringsListPtr;
		os_free(avpIn);
	}else{
		return NULL;
	}
	/* Start listening for the messages comming on this port. If we receive some messages then intimate to observer by using callback function. */
	while(*bServerActive && !cSocket::cvCloseAllSessions){
		SOCK_INET_ADDR_STRUCT sClientSockAddr;
		socklen_t iClientSockAddrLen = sizeof(sClientSockAddr);
		int32 iRetVal = 0;
		/* Allocate the memory for recv buffer. Observer is responsible for releasing this memory.Ohterwise Memory leak will happen. Be Careful. */
		int8* acpRecvBuf = (int8*)os_malloc(iRecvBufSize);
		if(NULL == acpRecvBuf){
			close(iSockID);
			return NULL;
		}

		/* Start looking for the messages on the above socket. */
		iRetVal = recvfrom(iSockID,acpRecvBuf,iRecvBufSize,0,(SOCK_ADDR_STRUCT*)&sClientSockAddr,&iClientSockAddrLen);
		/* If we got error. Then something is wrong. Keep on trying. */
		if(iRetVal <= 0){
			if(acpRecvBuf){
				os_free(acpRecvBuf);
				acpRecvBuf = NULL;
			}
			continue;
		}
		matchingStringFound = false;
		//LOGD("cSocket::UDPBlockingListenCall Got UDP Message = %s",acpRecvBuf);
		/* Check for the Matching String if cvUDPFilterStringList is set. And only proceed further it that string is
		  contained inside the UDP message received. If cvUDPFilterString is not set then no problem. Go ahead with
		  out any check.*/
		//LOGD("cSocket::ReceiveBroadcastMessages: Number of elements inside filterStrListPtr = %d",filterStrListPtr->GetNumberOfElements());
		UDPFilterString filterString;
		for(count=1; count <= filterStrListPtr->GetNumberOfElements(); count++){
		      os_memset(&filterString,'\0',sizeof(filterString));
		      //LOGD("cSocket::ReceiveBroadcastMessages: Calling filterStrListPtr->GetElementAtIndex with count = %d",count);
		      filterStrListPtr->GetElementAtIndex(filterString,count);
		      //LOGD("cSocket::ReceiveBroadcastMessages: Retrieved Filter String = [%s] with count = %d",filterString.FilterString,count);
		      if(NULL != os_strcasestr(acpRecvBuf,filterString.FilterString)){
			      //LOG_CONSOLE("cSocket::UDPBlockingListenCall matching string [%s]is found in the UDP Message received. Hence informing to the observer.",filterString.FilterString);
			      matchingStringFound = true;
			      break;
		      }
		}

		/* If none of the filtering strings matched then no need to inform the oberserver... Just ignore.. */
		if(false == matchingStringFound && (0 < filterStrListPtr->GetNumberOfElements())){
		      //LOG_CONSOLE("cSocket::UDPBlockingListenCall none of the filtering strings matched to the received message..Hence not informing to the observer.");
		      if(acpRecvBuf){
			      os_free(acpRecvBuf);
			      acpRecvBuf = NULL;
		      }
		      continue;
		}
		LOGD("cSocket::ReceiveBroadcastMessages Matching string found. Hence intimating to listener");
		/* Create the sturucture which holds the observer pointer and the client socket ID. The memory for this
		  structure should be allocated dynamically and pass this to Create_New_Thread function such that the
		  same pointer will be passed to the thread registered function and inside that function the memory for
		  of this structure will be deallocated.*/
		cvThreadStruct *threadStruct = (cvThreadStruct*)os_malloc(sizeof(cvThreadStruct));
		if(NULL == threadStruct){
			close(iSockID);
			if(acpRecvBuf){
				os_free(acpRecvBuf);
				acpRecvBuf = NULL;
			}
			return NULL;
		}
		os_memset(threadStruct,'\0',sizeof(cvThreadStruct));
		threadStruct->Observer = observer;
		threadStruct->SockID = iSockID;
		threadStruct->SockActivity = UDP_MESSAGE_RECEIVED;
		if(iClientSockAddrLen){
			int8 *cpTemp = inet_ntoa(sClientSockAddr.sin_addr);
			os_strncpy(threadStruct->ClientAddr,cpTemp,sizeof(threadStruct->ClientAddr));
			threadStruct->ClientPort = ntohs(sClientSockAddr.sin_port);
		}
		threadStruct->RecvBufPtr = acpRecvBuf;
		threadStruct->RecvBufLen = iRetVal;
		os_strncpy(threadStruct->SelfAddr,selfAddr,sizeof(threadStruct->SelfAddr));
		threadStruct->SelfPort = selfPort;
		/* Create the new thread for each connected clients. */
		if(-1 == Create_New_Thread(cSocket::AcceptCallback,(void*)threadStruct,THREAD_PRIORITY_IS_NORMAL)){
			os_free(threadStruct);
			if(acpRecvBuf){
				os_free(acpRecvBuf);
				acpRecvBuf = NULL;
			}
			continue;
		}
	}
	Terminate_Thread();
	return NULL;
}

void* cSocket::AcceptCallback(void *avpIn)
{
	if(avpIn){
		SocketObserverParams sockObsParms;
		TCPSocketInfo tcpSockInfo;
		UDPSocketInfo udpSockInfo;
		cvThreadStruct *threadStruct = (cvThreadStruct*)avpIn;
		if(threadStruct){
			/* Initialize all the structures with zeros. Just for safety. */
			os_memset(&sockObsParms,0,sizeof(sockObsParms));
			os_memset(&tcpSockInfo,0,sizeof(tcpSockInfo));
			os_memset(&udpSockInfo,0,sizeof(udpSockInfo));
			sockObsParms.SockActivity = threadStruct->SockActivity;
			if(TCP_CLIENT_CONNECTED == threadStruct->SockActivity){
				tcpSockInfo.SockID = threadStruct->SockID;
				os_strncpy(tcpSockInfo.SelfIPPtr,threadStruct->SelfAddr,sizeof(tcpSockInfo.SelfIPPtr));
				tcpSockInfo.SelfPort = threadStruct->SelfPort;
				os_strncpy(tcpSockInfo.PeerIPPtr,threadStruct->ClientAddr,sizeof(tcpSockInfo.PeerIPPtr));
				tcpSockInfo.PeerPort = threadStruct->ClientPort;
				sockObsParms.TCPSockInfoPtr = &tcpSockInfo;
			}else if(UDP_MESSAGE_RECEIVED == threadStruct->SockActivity){
				udpSockInfo.SockID = threadStruct->SockID;
				os_strncpy(udpSockInfo.SelfIPPtr,threadStruct->SelfAddr,sizeof(udpSockInfo.SelfIPPtr));
				udpSockInfo.SelfPort = threadStruct->SelfPort;
				os_strncpy(udpSockInfo.PeerIPPtr,threadStruct->ClientAddr,sizeof(udpSockInfo.PeerIPPtr));
				udpSockInfo.PeerPort = threadStruct->ClientPort;
				udpSockInfo.RecvBufPtr = (int8*)threadStruct->RecvBufPtr;
				udpSockInfo.RecvBufLen = threadStruct->RecvBufLen;
				sockObsParms.UDPSockInfoPtr = &udpSockInfo;
			}
			cSocketObserver *observer = threadStruct->Observer;
			os_free(threadStruct);
			if(observer){
				observer->HandleSocketActivity(sockObsParms);
			}
		}
	}
	//Terminate_Thread();
	return NULL;
}

int32 cSocket::Send(IN const SOCKET aSockID, IN const int8* const aBuf, IN const uint32 aBufLen)
{
	int32 iRetVal = 0;
	uint32 iSendBytes = 0;
	while((-1 != iRetVal) && (iSendBytes < aBufLen)){
		iRetVal = send(aSockID, aBuf+iSendBytes, (aBufLen - iSendBytes), 0);
		//iRetVal = write(aSockID, aBuf+iSendBytes, (aBufLen - iSendBytes);
		while ((-1 == iRetVal) && (errno == EAGAIN || errno == EINTR)) {
			os_sleep(50);
			iRetVal=send(aSockID,aBuf+iSendBytes,(aBufLen-iSendBytes),0);
			//iRetVal = write(aSockID, aBuf+iSendBytes, (aBufLen - iSendBytes);
		}
		if(iRetVal > 0){
			iSendBytes += iRetVal;
		}
	}
	return iSendBytes;
}

int32 cSocket::Receive(IN const SOCKET aSockID, OUT int8* const aBuf, IN const uint32 aBufLen)
{
	int32 iRecvBytes = 0;
	uint32 totalBytesReceived = 0;
	//iRecvBytes = read(aSockID, aBuf, aBufLen);
	while(totalBytesReceived < aBufLen){
		iRecvBytes = recv(aSockID, aBuf+totalBytesReceived, aBufLen-totalBytesReceived, OS_NO_SIGNAL);
		while ((-1 == iRecvBytes) && (errno == EAGAIN || errno == EINTR)) {
			os_sleep(50);
			iRecvBytes = recv(aSockID, aBuf, aBufLen, OS_NO_SIGNAL);
			//iRecvBytes = read(aSockID, aBuf, aBufLen);
		}
		if(-1 == iRecvBytes){
			//perror("Socket Recv Error:");
			return iRecvBytes;
		}else if(0 == iRecvBytes){
			return totalBytesReceived; // If iRecvBytes is zero means then the connection might have closed by peer.. so we can return from here itself.
		}
		//LOGD("cSocket::Receive receiving some more data from socket.. Received = %d Yet to Received is %d",iRecvBytes,aBufLen-totalBytesReceived);
		totalBytesReceived += iRecvBytes;
		iRecvBytes = 0;
	}
	return totalBytesReceived;
}

int32 cSocket::SendTo(IN const SOCKET aSockID, IN const int8* const aBuf, IN const uint32 aBufLen,
		      IN const int8* const aAddr, IN const uint32 aPort)
{
	int32 iRetVal = 0;
	uint32 iSendBytes = 0;
	SOCK_INET_ADDR_STRUCT sSockAddr;
	os_memset(&(sSockAddr), 0, sizeof(sSockAddr));
	sSockAddr.sin_family = AF_INET;
	sSockAddr.sin_addr.s_addr = inet_addr(aAddr);
	sSockAddr.sin_port = htons(aPort);

	while((-1 != iRetVal) && (iSendBytes < aBufLen)){
		iRetVal = sendto(aSockID, aBuf+iSendBytes, (aBufLen - iSendBytes), 0, (SOCK_ADDR_STRUCT *)&sSockAddr,sizeof(sSockAddr));
		while ((-1 == iRetVal) && (errno == EAGAIN || errno == EINTR)) {
			os_sleep(50);
			iRetVal=sendto(aSockID,aBuf+iSendBytes,(aBufLen-iSendBytes),0, (SOCK_ADDR_STRUCT *)&sSockAddr,sizeof(sSockAddr));
		}
		if(iRetVal > 0){
			iSendBytes += iRetVal;
		}
	}
	return iSendBytes;
}

int32 cSocket::ReceiveFrom(IN const SOCKET aSockID, OUT int8* const aBuf, IN const uint32 aBufLen,
			   OUT int8* const aAddr, OUT uint32& aPort)
{
	SOCK_INET_ADDR_STRUCT sSockAddr;
	int32 iRecvBytes = 0;
	socklen_t iSockAddrLen = sizeof(sSockAddr);
	iRecvBytes = recvfrom(aSockID, aBuf, aBufLen, MSG_NOSIGNAL,(SOCK_ADDR_STRUCT *)&sSockAddr,&iSockAddrLen);
	while ((-1 == iRecvBytes) && (errno == EAGAIN || errno == EINTR)) {
		os_sleep(50);
		iRecvBytes = recvfrom(aSockID, aBuf, aBufLen, MSG_NOSIGNAL,(SOCK_ADDR_STRUCT *)&sSockAddr,&iSockAddrLen);
	}
	if (iRecvBytes > 0) {
		if (aAddr){
			os_strncpy(aAddr,inet_ntoa(sSockAddr.sin_addr),MAXIMUM_IP_SIZE);
		}
		aPort = ntohs(sSockAddr.sin_port);
	}
	return iRecvBytes;
}

void cSocket::CloseSocket(const SOCKET aSockID)
{
	/* Disallowing further read and write operations on this socket. */
	if(-1 == shutdown(aSockID,SHUT_RDWR)){
	  /* No need to worry about the failure of the above command. Take it easy :). Just log this for debugging. */
	}
	/* Closing the socket and releasing the socket discriptor to OS */
	if(-1 == close(aSockID)){
	  /* No need to worry about the failure of the above command. Take it easy :). Just log this for debugging.  */
	}
}

void cSocket::StopSocketServer()
{
	cvSocketServerActive = false;
}

void cSocket::CloseConnection(void)
{
	/* Set the ServerActive flag to false. Such that the above infinate accept call while loop will be broken. */
	cvSocketServerActive = false;
	/* Disallowing further read and write operations on the main socket. */
	if(-1 == shutdown(cvSockID,SHUT_RDWR)){
	  /* No need to worry about the failure of the above command. Take it easy :). Just log this for debugging. */
	}
	/* Closing the socket and releasing the socket discriptor to OS */
	if(-1 == close(cvSockID)){
	  /* No need to worry about the failure of the above command. Take it easy :). Just log this for debugging.  */
	}
}

void cSocket::CloseAllSessions(void)
{
	cSocket::cvCloseAllSessions = true;
	int32 i = 0;
	while(-1 != cSocket::cvConnectedSockIDs[i]){
		/* Disallowing further read and write operations on the main socket. */
		if(-1 == shutdown(cSocket::cvConnectedSockIDs[i],SHUT_RDWR)){
		  /* No need to worry about the failure of the above command. Take it easy :). Just log this for debugging. */
		}
		/* Closing the socket and releasing the socket discriptor to OS */
		if(-1 == close(cSocket::cvConnectedSockIDs[i])){
		  /* No need to worry about the failure of the above command. Take it easy :). Just log this for debugging.  */
		}
		cSocket::cvConnectedSockIDs[i] = -1;
		i++;
	}
	i = 0;
}

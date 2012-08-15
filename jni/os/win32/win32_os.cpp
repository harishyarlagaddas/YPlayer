   /* This is the windows spcific implementation of the generic os functions
      declarated inside the os.h file. Actually all the upper layers will
      use the functions declared in the os.h file such that there will not
      be any platform dependency and based on the platform all the functions
      declared in the os.h file will be implemented in the platform specific
      files and will be linked to the corresponding build. This way we can
      achieve the os compatibility.
   */

#include "win32_os.h"
#include "os.h"

/********************************************************************************
******************* File related Operations BEGIN *******************************
*********************************************************************************/

/********************************************************************************
******************* File related Operations END  ********************************
*********************************************************************************/

/********************************************************************************
******************* Mutex related Functions***** ********************************
*********************************************************************************/

int32 os_mutex_initialize(IN os_mutex *aOSMutex, IN os_mutex_attribute *aOSMutexAttr)
{
  // TODO-WIN32 return pthread_mutex_init(aOSMutex, aOSMutexAttr);
	return 0;
}

int32 os_mutex_destroy(IN os_mutex *aOSMutex)
{
  // TODO-WIN32 return pthread_mutex_destroy(aOSMutex);
	return 0;
}

int32 os_mutex_lock(IN os_mutex *aOSMutex)
{
  // TODO-WIN32 return pthread_mutex_lock(aOSMutex);
	return 0;
}

int32 os_mutex_unlock(IN os_mutex *aOSMutex)
{
  // TODO-WIN32 return pthread_mutex_unlock(aOSMutex);
	return 0;
}

int32 os_mutex_trylock(IN os_mutex *aOSMutex)
{
  // TODO-WIN32 return pthread_mutex_trylock(aOSMutex);
	return 0;
}

/********************************************************************************
******************* Mutex related Functions END *********************************
*********************************************************************************/

/********************************************************************************
******************* Thread related functions ************************************
*********************************************************************************/

int32 Create_New_Thread(void* (*aInitFuncPtr) (void *), void const *aFuncArg, const uThreadPriority aThreadPriority)
{
	// TODO-WIN32
#if 0
	pthread_t iThreadID;
	uint32 iDefaultStackSize = 128*1024;
	pthread_attr_t sAttribute;
	struct sched_param sSchedulerParam;
	int8 cRetVal = 0;

	/* Initialize the pthread related strucutres. */
	pthread_attr_init(&sAttribute);
	os_memset(&sSchedulerParam, 0, sizeof(sSchedulerParam	));

	/* Set the scheduler priority of this particular thread. */
	sSchedulerParam.sched_priority = aThreadPriority;
	pthread_attr_setschedparam(&sAttribute, &sSchedulerParam);

	/* Set the stack size for this thread. */
	pthread_attr_setstacksize(&sAttribute,iDefaultStackSize);

	/* create the thread with the above initialized attributes. */
	cRetVal = pthread_create(&iThreadID, &sAttribute, aInitFuncPtr, (void*)aFuncArg);

	/* If we get the return value other than 0, then it means there is some error. Handling it accordingly. */
	if (cRetVal) {
		/* If the error is EAGAIN, then we can wait for some amount of time and try again. */
		if (cRetVal == EAGAIN) {
			os_sleep(250);
			cRetVal = pthread_create(&iThreadID, &sAttribute, aInitFuncPtr, (void*)aFuncArg);

			/* Even after waiting some time, if it fails then we can't do much. Just error out. */
			if(cRetVal){
				iThreadID = -1;
			}
		}
		/* If the error is not EAGAIN, then it might have failed because fo priority. So now just try with default priority. */
		else if (cRetVal != EAGAIN) {
			cRetVal = pthread_create(&iThreadID, NULL, aInitFuncPtr, (void*)aFuncArg);
			/* Even if it fails with default priority, then there is not much we can do it. Just error out. */
			if(cRetVal){
				iThreadID = -1;
			}
		}
	}
	/* Destroy the attribute. */
	pthread_attr_destroy(&sAttribute);

	if(-1 != (int32)iThreadID){
		/* Resources of this thread can be reclaimed upon thread termination. */
		pthread_detach(iThreadID);
	}
	/* Return the thread id to the calling function. */
	return iThreadID;
#endif
	return 0;
}

int32 Terminate_Thread()
{
	// TODO-WIN32 why it is not returning in linux? pthread_exit(NULL);
	return 0;
}

/********************************************************************************
******************* Thread related functions END ********************************
*********************************************************************************/

/********************************************************************************
******************* MISCELENEOUS WIN32 Util Functions BEGIN *********************
*********************************************************************************/
int32 os_get_NumberOfActiveEthernetInterfaces()
{
	struct in_addr *pIPaddress=NULL;
	struct hostent *pHostEntry;
	char strHostname[256];
	int bLocalhostAdded = false,nInterfaceCount=0;
	char ipBuffer [17]={0};
	int error = 0;

	if(!gethostname(strHostname,sizeof(strHostname)))
	{
		pHostEntry = gethostbyname(strHostname);
	}
	else
	{
		error = WSAGetLastError();
		// Log required
	}
	if (!pHostEntry) {
// Log required
	}

	while (nInterfaceCount<5) {
 		pIPaddress=(struct in_addr *)pHostEntry->h_addr_list[nInterfaceCount];
 		if (!pIPaddress) break;
		// only if all interfaces are disabled, 127.0.0.1 is in that list
		os_strncpy(ipBuffer,inet_ntoa(*pIPaddress),17);
		if (!os_strcmp(ipBuffer,"127.0.0.1"))
			bLocalhostAdded=TRUE;
		nInterfaceCount++;
 	}
	// add local IP
	if  (! bLocalhostAdded && (nInterfaceCount<5)) {
		nInterfaceCount++;
	}
	return nInterfaceCount;
}

int32 os_get_ActiveEthernetInterfaces(char *aAddressPtr,uint32 aSize)
{
	struct in_addr *pIPaddress=NULL;
	struct hostent *pHostEntry;
	char strHostname[256];
	int bLocalhostAdded = false,nInterfaceCount=0;
	char *pIPaddressArray=aAddressPtr,ipBuffer[MAXIMUM_IP_SIZE];

	gethostname(strHostname,sizeof(strHostname));
	pHostEntry = gethostbyname(strHostname);
	if (!pHostEntry) {
//		LOG_CONSOLE("os_get_NumberOfActiveEthernetInterfaces couldnot get interfaces.");
	}

	while (nInterfaceCount<5) {
 		pIPaddress=(struct in_addr *)pHostEntry->h_addr_list[nInterfaceCount];
 		if (!pIPaddress) break;
		// only if all interfaces are disabled, 127.0.0.1 is in that list
		os_strncpy(ipBuffer,inet_ntoa(*pIPaddress),MAXIMUM_IP_SIZE);
		if (!os_strcmp(ipBuffer,"127.0.0.1"))
			bLocalhostAdded=TRUE;
		os_strncpy(pIPaddressArray,ipBuffer,MAXIMUM_IP_SIZE);
		nInterfaceCount++;
		pIPaddressArray += MAXIMUM_IP_SIZE;
 	}
	// add local IP
	if  (! bLocalhostAdded && (nInterfaceCount<5)) {
		os_strncpy(pIPaddressArray,"127.0.0.1",MAXIMUM_IP_SIZE);
		nInterfaceCount++;
	}
	return nInterfaceCount;
}

int32 os_set_SocketOptions(const SOCKET aSockID, SockOptions aSockOption, SockOptValue *aSockOptVal1, SockOptValue *aSockOptVal2)
{
#if 0 // TODO-WIN32
	int32 iRetVal = 0;
	switch(aSockOption)
	{
		case SEND_TIMEOUT:
		case RECEIVE_TIMEOUT:
			break;
		case ADD_MEMBERSHIP:
		{
			int8 *cpOwnAddr = (int8*)aSockOptVal1->vpPtr;
			int8 *cpMemberAddr = (int8*)aSockOptVal2->vpPtr;
			struct ip_mreq sMembershipReqStruct;

			os_memset(&sMembershipReqStruct,'\0',sizeof(sMembershipReqStruct));
			if(NULL == cpOwnAddr){
				sMembershipReqStruct.imr_interface.s_addr = INADDR_ANY;
			}else{
				sMembershipReqStruct.imr_interface.s_addr = inet_addr(cpOwnAddr);
			}
			sMembershipReqStruct.imr_multiaddr.s_addr = inet_addr(cpMemberAddr);

			iRetVal =  setsockopt(aSockID, IPPROTO_IP, IP_ADD_MEMBERSHIP,(int8*)&sMembershipReqStruct, sizeof(sMembershipReqStruct));
			if(-1 == iRetVal){
				  perror("setsockopt Error:");
			}
		}
		break;
		case MULTICAST_INTERFACE:
		{
			int8 *cpAddr = (int8*)aSockOptVal1->vpPtr;
			struct in_addr sAddrStruct;
			sAddrStruct.s_addr = inet_addr(cpAddr);

			iRetVal = setsockopt(aSockID, IPPROTO_IP, IP_MULTICAST_IF, (char*)&sAddrStruct, sizeof(sAddrStruct));
			if(-1 == iRetVal){
			}
		}
		break;
		case TTL:
		{
			int32 iTTL = aSockOptVal1->iValue;
			iRetVal = setsockopt(aSockID, IPPROTO_IP, IP_MULTICAST_TTL,(char*)&iTTL, sizeof(iTTL));
			if(-1 == iRetVal){
			}
		}
		break;
	}
	return iRetVal;
#endif
	return 0;
}

/********************************************************************************
******************* INOTIFY Related Functions END.  *****************************
*********************************************************************************/

/********************************************************************************
******************* File Find Related Functions BEGIN ***************************
*********************************************************************************/
os_dirhandle os_find_start(int8 *aDirName)
{
// TODO
	return 0;
}
/********************************************************************************
******************* MISCELENEOUS WIN32 Util Functions END ***********************
*********************************************************************************/

/********************************************************************************
******************* OS Related Functions BEGIN **********************************
*********************************************************************************/
void os_sleep(uint32 aMilliSec)
{
	/*
	struct timespec timeStruct;
	timeStruct.tv_sec = aMilliSec/1000;
	timeStruct.tv_nsec = (aMilliSec % 1000) * 1000000;
	nanosleep(&timeStruct,NULL);
	*/
}
/********************************************************************************
******************* OS Related Functions END **********************************
*********************************************************************************/


/********************************************************************************
*********************** Signal handler related functions ************************
*********************************************************************************/
void os_signal_register(int32 aSignal, void(*fp)(int))
{
	signal(aSignal,fp);
}
/********************************************************************************
******************* Signal handler related function END *************************
*********************************************************************************/


int8* os_get_date()
{
	return NULL;
	//TODO
}

int8* os_ctime(os_time aTimeVal)
{
	return NULL;
	//TODO
}

int32 os_get_thread_id()
{
	return (int32)0;
	//TODO
}

int32 os_get_time()
{
	return 0;
	//TODO
}

ReturnStatus os_file_stat(int8 *aFileName,os_file_stat_buf *aStatBuf)
{
	return SUCCESS;
	//TODO
}

os_inotify_fd os_inotify_init(void)
{
	return -1;
	//TODO
}

os_inotify_watch_fd os_inotify_add_watch(os_inotify_fd aFd, int8 *aPathName, os_inotify_mask aMask)
{
	return -1;
	//TODO
}

os_file_find_struct os_find_next(os_dirhandle aDirHandle)
{
	os_file_find_struct fileFindStruct;
	return fileFindStruct;
	//TODO
}

   /* This is the linux spcific implementation of the generic os functions
      declarated inside the os.h file. Actually all the upper layers will
      use the functions declared in the os.h file such that there will not
      be any platform dependency and based on the platform all the functions
      declared in the os.h file will be implemented in the platform specific
      files and will be linked to the corresponding build. This way we can
      achieve the os compatibility.
   */

#include "linux_os.h"
#include "../os.h"
#include "../../common/log/log.h"

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
  return pthread_mutex_init(aOSMutex, aOSMutexAttr);
}

int32 os_mutex_destroy(IN os_mutex *aOSMutex)
{
  return pthread_mutex_destroy(aOSMutex);
}

int32 os_mutex_lock(IN os_mutex *aOSMutex)
{
  return pthread_mutex_lock(aOSMutex);
}

int32 os_mutex_unlock(IN os_mutex *aOSMutex)
{
  return pthread_mutex_unlock(aOSMutex);
}

int32 os_mutex_trylock(IN os_mutex *aOSMutex)
{
  return pthread_mutex_trylock(aOSMutex);
}
/********************************************************************************
******************* Mutex related Functions END *********************************
*********************************************************************************/

/********************************************************************************
******************* Thread related functions ************************************
*********************************************************************************/
int32 Create_New_Thread(void* (*aInitFuncPtr) (void *), void const *aFuncArg, const uThreadPriority aThreadPriority)
{
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
}

void Terminate_Thread()
{
	pthread_exit(NULL);
}
/********************************************************************************
******************* Thread related functions END ********************************
*********************************************************************************/

/********************************************************************************
******************* MISCELENEOUS Linux Util Functions BEGIN *********************
*********************************************************************************/
int32 os_get_NumberOfActiveEthernetInterfaces()
{
	struct ifconf sIfConf;
	struct ifreq *sIfReq = NULL;
	SOCKET iSocket;
	int32 iCount = 0, i=0, iNoOfActiveInterfaces = 0, iRetVal = -1;

	os_memset(&sIfConf,'\0',sizeof(sIfConf));
	iSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (iSocket < 0) {
		LOGE("os_get_NumberOfActiveEthernetInterfaces Unable to Create the Socket Error: %d, errno = %d",iSocket,errno);
		return FAILURE;
	}

	  /* Actually we don't know how many interfaces are present at this time. So try to ask for the first
	    5 interfaces at one time. If kernel returns all 5, it means there might be more than 5, so now
	    again ask for 10 and repeat so on, until kernel returns less than what we ask. */
	do{
		iCount++;
		if(sIfConf.ifc_buf){
			os_free(sIfConf.ifc_buf);
		}
		sIfConf.ifc_buf = (int8*)os_malloc(iCount * 5 * sizeof(struct ifreq));
		sIfConf.ifc_len = iCount * 5 * sizeof(struct ifreq);
		if (ioctl(iSocket, SIOCGIFCONF, &sIfConf) < 0) {
			close(iSocket);
			iRetVal = FAILURE;
			goto Exit;
		}
	}while(sIfConf.ifc_len == (int32)(iCount * 5 * sizeof(struct ifreq)));

	iCount = sIfConf.ifc_len / sizeof(struct ifreq);
	sIfReq = (struct ifreq*)sIfConf.ifc_buf;
	for(i=0; i< iCount; i++){
		if(ioctl(iSocket, SIOCGIFFLAGS, &sIfReq[i]) < 0){
			close(iSocket);
			iRetVal = iNoOfActiveInterfaces;
			goto Exit;
		}
		if(sIfReq[i].ifr_flags & IFF_UP){
			iNoOfActiveInterfaces++;
		}
	}
	iRetVal = iNoOfActiveInterfaces;
Exit:
	if(sIfConf.ifc_buf){
		os_free(sIfConf.ifc_buf);
	}
	return iRetVal;
}

int32 os_get_ActiveEthernetInterfaces(char *aAddressPtr,uint32 aSize)
{
	struct ifconf sIfConf;
	struct ifreq *sIfReq = NULL;
	SOCKET iSocket;
	int32 iCount = 0, i=0, iNoOfActiveInterfaces = 0,iRetVal = -1;

	os_memset(&sIfConf,'\0',sizeof(sIfConf));
	iSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (iSocket < 0) {
		return FAILURE;
	}

	  /* Actually we don't know how many interfaces are present at this time. So try to ask for the first
	    5 interfaces at one time. If kernel returns all 5, it means there might be more than 5, so now
	    again ask for 10 and repeat so on, until kernel returns less than what we ask. */
	do{
		iCount++;
		if(sIfConf.ifc_buf){
			os_free(sIfConf.ifc_buf);
		}
		sIfConf.ifc_buf = (int8*)os_malloc(iCount * 5 * sizeof(struct ifreq));
		sIfConf.ifc_len = iCount * 5 * sizeof(struct ifreq);
		if (ioctl(iSocket, SIOCGIFCONF, &sIfConf) < 0) {
			close(iSocket);
			iRetVal = FAILURE;
			goto Exit;
		}
	}while(sIfConf.ifc_len == (int32)(iCount * 5 * sizeof(struct ifreq)));

	iCount = sIfConf.ifc_len / sizeof(struct ifreq);
	if((aSize/MAXIMUM_IP_SIZE) < (uint32)iCount){
		iCount = aSize/MAXIMUM_IP_SIZE;
	}
	sIfReq = (struct ifreq*)sIfConf.ifc_buf;
	for(i=0; i< iCount; i++){
		if(ioctl(iSocket, SIOCGIFFLAGS, &sIfReq[i]) < 0){
			close(iSocket);
			iRetVal = iNoOfActiveInterfaces;
			goto Exit;
		}
		if(sIfReq[i].ifr_flags & IFF_UP){
			os_strncpy((aAddressPtr+(i*MAXIMUM_IP_SIZE)),inet_ntoa(((struct sockaddr_in*)&sIfReq[i].ifr_addr)->sin_addr),MAXIMUM_IP_SIZE);
			iNoOfActiveInterfaces++;
		}
	}
	iRetVal = iNoOfActiveInterfaces;
Exit:
	if(sIfConf.ifc_buf){
		os_free(sIfConf.ifc_buf);
	}
	return iRetVal;
}

int32 os_set_SocketOptions(const SOCKET aSockID, SockOptions aSockOption, SockOptValue *aSockOptVal1, SockOptValue *aSockOptVal2)
{
	int32 iRetVal = 0;
	switch(aSockOption)
	{
		case SEND_TIMEOUT:
			break;
		case RECEIVE_TIMEOUT:
		{
			struct timeval timeStruct;
			int32 val = aSockOptVal1->iValue;
			os_memset(&timeStruct,0,sizeof(timeStruct));
			/* Assuming incomming value comes in milli seconds, we need to covert it to the correct seconds and micro seconds
			   accordingly. */
			timeStruct.tv_sec = val/1000;
			timeStruct.tv_sec = (val % 1000) * 1000;
			iRetVal =  setsockopt(aSockID, SOL_SOCKET, SO_RCVTIMEO,(int8*)&timeStruct, sizeof(timeStruct));
			if(-1 == iRetVal){
				  perror("RECEIVE_TIMEOUT setsockopt Error:");
			}
		}
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
				  perror("ADD_MEMBERSHIP setsockopt Error:");
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
				perror("MULTICAST_INTERFACE setsockopt Error:");
			}
		}
		break;
		case TTL:
		{
			int32 iTTL = aSockOptVal1->iValue;
			iRetVal = setsockopt(aSockID, IPPROTO_IP, IP_MULTICAST_TTL,(char*)&iTTL, sizeof(iTTL));
			if(-1 == iRetVal){
				perror("TTL setsockopt Error:");
			}
		}
		break;
		case REUSE_ADDR:
		{
			int32 reuseAddr = 1;
			iRetVal = setsockopt(aSockID, SOL_SOCKET, SO_REUSEADDR,(char*)&reuseAddr, sizeof(reuseAddr));
			if(-1 == iRetVal){
				perror("REUSE_ADDR setsockopt Error:");
			}
		}
		break;
	}
	return iRetVal;
}

ReturnStatus os_get_SocketAddress(const SOCKET aSockID, int8 *aIPAddr, int32* aPortNo)
{
	SOCK_INET_ADDR_STRUCT sockAddr;
	int32 len;
	if(-1 != getsockname(aSockID,(SOCK_ADDR_STRUCT*)&sockAddr,(socklen_t*)&len)){
		if(NULL != aIPAddr){
			memcpy((void*)aIPAddr,(void*)inet_ntoa(sockAddr.sin_addr),16);
		}
		if(NULL != aPortNo){
			*aPortNo = sockAddr.sin_port;
		}
		return SUCCESS;
	}else{
		return FAILURE;
	}
}
/********************************************************************************
******************* MISCELENEOUS Linux Util Functions END ***********************
*********************************************************************************/

/********************************************************************************
******************* OS Related Functions BEGIN **********************************
*********************************************************************************/
void os_sleep(uint32 aMilliSec)
{
	struct timespec timeStruct;
	timeStruct.tv_sec = aMilliSec/1000;
	timeStruct.tv_nsec = (aMilliSec % 1000) * 1000000;
	nanosleep(&timeStruct,NULL);
}
/********************************************************************************
******************* OS Related Functions END **********************************
*********************************************************************************/

/********************************************************************************
******************* INOTIFY Related Functions BEGIN *****************************
*********************************************************************************/
os_inotify_fd os_inotify_init(void)
{
#ifdef INOTIFY
	return inotify_init();
#else
	return -1;
#endif
}

os_inotify_watch_fd os_inotify_add_watch(os_inotify_fd aFd, int8 *aPathName, os_inotify_mask aMask)
{
#ifdef INOTIFY
	return inotify_add_watch(aFd,aPathName,aMask);
#else
	return -1;
#endif
}

int32 os_inotify_rm_watch(os_inotify_fd aFd, os_inotify_watch_fd aWatchFd)
{
#ifdef INOTIFY
	return inotify_rm_watch(aFd,aWatchFd);
#else
	return -1;
#endif
}
/********************************************************************************
******************* INOTIFY Related Functions END.  *****************************
*********************************************************************************/

/********************************************************************************
******************* File Find Related Functions BEGIN ***************************
*********************************************************************************/
os_dirhandle os_find_start(int8 *aDirName)
{
	if(NULL == aDirName){
		return NULL;
	}
	return opendir(aDirName);
}

os_file_find_struct os_find_next(os_dirhandle aDirHandle)
{
	os_file_find_struct fileFindStruct;
	os_memset(&fileFindStruct,0,sizeof(fileFindStruct));

	/* Structure used for reading the next file.*/
	struct dirent *dirStruct;

	if(NULL == aDirHandle){
		/* If the Directory Handle is NULL, No need to do much. Just return empty structure. */
		return fileFindStruct;
	}

	dirStruct = readdir(aDirHandle);
	if(NULL == dirStruct){
		/* We might have reached to the end of the file list in directory or might have got error. For now if NULL is returned
		   treat it as end of list. In future if it became necessary to identify the error case then check for errno flag to
		   check whether NULL is returned becase of end of file list or because of some error. */
		return fileFindStruct;
	}

	/* Fill the fileFindStruct according to the returned structure form read dir call. */
	os_strncpy(fileFindStruct.FileName,dirStruct->d_name,sizeof(fileFindStruct.FileName));
	/* In old systems d_type variable is not supported. But here I used it. In case if you face any issue then use lstat to get the
	   type of the file. Here inorder to avoid the expense of calling lstat. */
	fileFindStruct.IsDirectory = (dirStruct->d_type == DT_DIR)?1:0;
	fileFindStruct.IsSoftLink = (dirStruct->d_type == DT_LNK)?1:0;

	/* Completed the filling of the structure. we can return it now.*/
	return fileFindStruct;
}

void os_find_close(os_dirhandle aDirHandle)
{
	if(NULL == aDirHandle){
		return;
	}
	closedir(aDirHandle);
}
/********************************************************************************
******************* File Find Related Functions END ***************************
*********************************************************************************/

/********************************************************************************
******************* File File Related Functions Start ***************************
*********************************************************************************/
int32 os_gettime();
ReturnStatus os_file_stat(int8 *aFileName,os_file_stat_buf *aStatBuf)
{
	if(NULL == aFileName){
		return FAILURE;
	}
	struct stat statBuf;
	os_memset(&statBuf,0,sizeof(statBuf));
	if(-1 == stat(aFileName,&statBuf)){
		return FAILURE;
	}else{
		aStatBuf->Size = statBuf.st_size;
		aStatBuf->LastAccessTime = statBuf.st_atime;
		aStatBuf->LastModificationTime = statBuf.st_mtime;
		return SUCCESS;
	}
}

/********************************************************************************
******************* File File Related Functions END ***************************
*********************************************************************************/

/********************************************************************************
******************* Time Related Functions Start ********************************
*********************************************************************************/

int8* os_ctime(os_time aTimeVal)
{
	os_time timeVal = aTimeVal;
	return ctime(&timeVal);
}

os_time_struct* os_get_time()
{
	struct timeval timeVal;
	os_memset(&timeVal,0,sizeof(timeVal));
	if(-1 == gettimeofday(&timeVal,NULL)){
		return NULL;
	}else{
		return localtime(&(timeVal.tv_sec));
	}
}

int8* os_get_date()
{
	struct timeval timeStruct;
	os_memset(&timeStruct,0,sizeof(timeStruct));
	if(-1 == gettimeofday(&timeStruct,NULL)){
		return NULL;
	}else{
		return os_ctime(timeStruct.tv_sec);
	}
}

int32 os_get_thread_id()
{
	pid_t tid;
#ifndef ANDROID
    tid = syscall(SYS_gettid);
#endif
	return (int32)tid;
}

/********************************************************************************
******************* Time Related Functions END **********************************
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

/********************************************************************************
******************* Semaphore related functions BEGIN   *************************
*********************************************************************************/

int32 os_semaphore_init(os_semaphore *aSem, int32 aShared, uint32 aValue)
{
	return sem_init(aSem,aShared,aValue);
}

int32 os_semaphore_wait(os_semaphore *aSem)
{
	return sem_wait(aSem);
}

int32 os_semaphore_post(os_semaphore *aSem)
{
	return sem_post(aSem);
}

int32 os_semaphore_close(os_semaphore *aSem)
{
	return sem_close(aSem);
}

/********************************************************************************
******************* Semaphore related functions EMD     *************************
*********************************************************************************/

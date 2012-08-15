
  /* This is the linux specific os header file in which all the linux related
     header files will be included. And in this file all the linux os related
     functions declarations will happen.
  */
#ifndef LINUX_OS_H_INCLUDED
#define LINUX_OS_H_INCLUDED

/* Commong header files */
#include <stdio.h>
#include <stdlib.h>
#include<unistd.h>
/* Variable argument related header files */
#include <stdarg.h>
/* Socket Related header files */
#include<sys/types.h>
#include<sys/socket.h>
#include<sys/un.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include <sys/param.h>
/* For getting the File Stats. */
#include <sys/stat.h>
/* Ethernet interfaces related header files. */
#include <sys/ioctl.h>
#include <net/if.h>
/* Header file for getting the system related errors */
#include <errno.h>
/* Thread related header files. */
#include <pthread.h>
/* Directory releated functions. */
#include <dirent.h>
/* String related operations. */
#include <string.h>
/* Time related operations. */
#include <sys/time.h>
/* GetTid related operations. */
#include <sys/syscall.h>
/* Signal handler related operations. */
#include <signal.h>
/* Semaphore related operations. */
#include <semaphore.h>

#ifdef INOTIFY
/* Inotify related header files. */
#include <sys/inotify.h>
#endif


#include "../../common/include/basic_datatypes.h"
/********************************************************************************
*********************** Memory related functions ********************************
*********************************************************************************/
#define os_snprintf snprintf
#define os_strncmp strncmp
#define os_strchr strchr
#define os_strstr strstr
#define os_strrchr strrchr
#define os_strcasestr strcasestr
#define os_strncasecmp strncasecmp
#define os_strcasecmp strcasecmp

/* Mutex related params. */
#define os_mutex pthread_mutex_t
#define os_mutex_attribute pthread_mutexattr_t
#define OS_MUTEX_INITIALIZER PTHREAD_MUTEX_INITIALIZER

/* Semaphore related params */
#define os_semaphore sem_t

int32 os_semaphore_init(os_semaphore *aSem, int32 aShared, uint32 aValue);
int32 os_semaphore_wait(os_semaphore *aSem);
int32 os_semaphore_post(os_semaphore *aSem);
int32 os_semaphore_close(os_semaphore *aSem);

/* File related operations */
#define os_delete_file(FILENAME) unlink(FILENAME)
#define os_sync sync

/* Thread related functions */
int32 Create_New_Thread(void* (*aInitFuncPtr) (void *), void const *aFuncArg, const uThreadPriority aThreadPriority);
void Terminate_Thread();

/* Mutex Related functions. */
 /* os_mutex - will be defined to corresponding os specific structure inside os specific include file
    os_mutex_attribute - will be defined to corresponding os specific structure inside os specific include file
    OS_MUTEX_INITIALIZER - will be defined to corresponding os specific value inside os specific include file
  */
int32 os_mutex_initialize(IN os_mutex *aOSMutex, IN os_mutex_attribute *aOSMutexAttr);
int32 os_mutex_destroy(IN os_mutex *aOSMutex);
int32 os_mutex_lock(IN os_mutex *aOSMutex);
int32 os_mutex_unlock(IN os_mutex *aOSMutex);
int32 os_mutex_trylock(IN os_mutex *aOSMutex);

/* MISCELENEOUS Linux Util Functions */
int32 os_get_NumberOfActiveEthernetInterfaces();
int32 os_get_ActiveEthernetInterfaces(char *aAddressPtr,uint32 aSize);

/* OS related Functions.*/
#define os_atoi atoi
#define os_atol atol
#define os_time time_t

typedef struct tm os_time_struct;
void os_sleep(uint32 aMilliSec);

/* Macros related to INOTIFY Events. */
#define OS_INOTIFY_CREATE 	IN_CREATE
#define OS_INOTIFY_DELETE 	IN_DELETE
#define OS_INOTIFY_DELETE_SELF 	IN_DELETE_SELF
#define OS_INOTIFY_MODIFY 	IN_MODIFY
#define OS_INOTIFY_MOVE_SELF 	IN_MOVE_SELF
#define OS_INOTIFY_MOVED_FROM 	IN_MOVED_FROM
#define OS_INOTIFY_MOVED_TO 	IN_MOVED_TO
#define OS_INOTIFY_CLOSE_WRITE	IN_CLOSE_WRITE
#define OS_INOTIFY_ALL_FLAGS 	IN_ALL_EVENTS
#define OS_IS_DIR		IN_ISDIR

typedef struct inotify_event os_inotify_event_struct;

/* File Find related Functions. */
#define os_dirhandle DIR*

/* Some generic Macros related to OS. */
#define OS_PATH_DELIMETER "/"
#define os_thread_id pthread_t

/* File related calls. */
#define os_read read
#define os_write write
#define os_close close

/* Socket related Options. */
#define SOCKET int32
#define OS_NO_SIGNAL MSG_NOSIGNAL

/* Signal handler related Operations. */
#define OS_SIGINT SIGINT
#define OS_SIGABRT SIGABRT
#define OS_SIGTERM SIGTERM
#define OS_SIGPIPE SIGPIPE

#endif /* LINUX_OS_H_INCLUDED */

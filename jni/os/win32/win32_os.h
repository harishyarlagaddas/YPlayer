
  /* This is the Win32 specific os header file in which all the Win32 related
     header files will be included. And in this file all the Win32 os related
     functions declarations will happen.
  */
#ifndef WIN32_OS_H_INCLUDED
#define WIN32_OS_H_INCLUDED
// no warning for "was declared deprecated"
#pragma warning( disable : 4996 )
/* Commong header files */
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include<io.h>
/* Variable argument related header files */
#include <stdarg.h>
/* Socket Related header files */
#include<sys/types.h>

#ifdef SOCKET
#undef SOCKET
#endif
#include <winsock2.h>

/* Header file for getting the system related errors */
#include <errno.h>
/* Thread related header files. */
//#include <pthread.h>
#include <signal.h>

#include "basic_datatypes.h"
/********************************************************************************
*********************** Memory related functions ********************************
*********************************************************************************/
#define os_snprintf _snprintf
#define os_strncmp strncmp
#define os_strchr strchr
#define os_strstr strstr
#define os_strrchr strrchr
#define os_strcasestr strstr //TODO-WIN32 strcasestr
#define os_strncasecmp strncmp //TODO-WIN32 strncasecmp

/* Mutex related params. */
#define os_mutex int //TODO-WIN32 pthread_mutex_t
#define os_mutex_attribute int //TODO-WIN32 pthread_mutexattr_t
#define OS_MUTEX_INITIALIZER int //TODO-WIN32 PTHREAD_MUTEX_INITIALIZER

/* File related operations */
#define os_delete_file(FILENAME) unlink(FILENAME)

#define os_dirhandle void*

#define InitSocket(done)	\
           WSADATA wsaData; \
           done=(0==WSAStartup(MAKEWORD( 2, 2 ),&wsaData));

/* Thread related functions */
int32 Create_New_Thread(void* (*aInitFuncPtr) (void *), void const *aFuncArg, const uThreadPriority aThreadPriority);
int32 Terminate_Thread();

/* Mutex Related functions. */
 /* os_mutex - will be defined to corresponding os specific structure inside os specific include file
    os_mutex_attribute - will be defined to corresponding os specific structure inside os specific include file
    OS_MUTEX_INITIALIZER - will be defined to corresponding os specific value inside os specific include file
  */
/* TODO
int32 os_mutex_initialize(IN os_mutex *aOSMutex, IN os_mutex_attribute aOSMutexAttr);
int32 os_mutex_destroy(IN os_mutex *aOSMutex);
int32 os_mutex_lock(IN os_mutex *aOSMutex);
int32 os_mutex_unlock(IN os_mutex *aOSMutex);
int32 os_mutex_trylock(IN os_mutex *aOSMutex);
*/
/* MISCELENEOUS Linux Util Functions */
int32 os_get_NumberOfActiveEthernetInterfaces();
int32 os_get_ActiveEthernetInterfaces(char *aAddressPtr,uint32 aSize);

/* Macros related to INOTIFY Events. */
#define OS_INOTIFY_CREATE 	1
#define OS_INOTIFY_DELETE 	2
#define OS_INOTIFY_DELETE_SELF 	4
#define OS_INOTIFY_MODIFY 	8
#define OS_INOTIFY_MOVE_SELF 	16
#define OS_INOTIFY_MOVED_FROM 	32
#define OS_INOTIFY_MOVED_TO 	64
#define OS_INOTIFY_CLOSE_WRITE	128
#define OS_INOTIFY_ALL_FLAGS 	256
#define OS_IS_DIR		512

#define os_time time_t
#define OS_PATH_DELIMETER "\\"

/* OS related Functions.*/
#define os_atoi atoi
void os_sleep(uint32 aMilliSec);

/* File related calls. */
#define os_read read
#define os_write write
#define os_close close

/* Signal handler related Operations. */
#define OS_SIGINT SIGINT
#define OS_SIGABRT SIGABRT
#define OS_SIGTERM SIGTERM
#define OS_SIGPIPE SIGBREAK

#define IN_UNMOUNT 1

typedef struct inotify_event {
    int      wd;       /* Watch descriptor */
    uint32 mask;     /* Mask of events */
    uint32 cookie;   /* Unique cookie associating related
                          events (for rename(2)) */
    uint32 len;      /* Size of 'name' field */
    char   *name;   /* Optional null-terminated name */
}os_inotify_event_struct;

#endif /* WIN32_OS_H_INCLUDED */

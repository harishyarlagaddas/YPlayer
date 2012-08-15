
/* This file incudes all the declartions of the generic os functions
   such that all other upper layer files can include this file for avoiding
   the compilation errors. Original implementation of these generic os functions
   will be done inside the platform specific files and the corresponding platform
   specific file will the linked while building the lib. This way we can support
   the multiple os.
   */
#ifndef OS_H_INCLUDED
#define OS_H_INCLUDED

#include "../common/include/basic_datatypes.h"
#include "../common/include/return_codes.h"

#ifdef LINUX
#include "linux/linux_os.h"
#endif
#ifdef WIN32
#include "win32/win32_os.h"
#endif

/* Android specific hash defines.... */
#ifdef ANDROID
#include "linux/linux_os.h"

#define SOCKET int32
#define size_t int32
#endif
/* Socket related params. */

typedef union{
  int32 iValue;
  void *vpPtr;
}SockOptValue;

typedef enum{
  SEND_TIMEOUT = 0,
  RECEIVE_TIMEOUT,
  ADD_MEMBERSHIP,
  MULTICAST_INTERFACE,
  TTL,
  REUSE_ADDR
}SockOptions;

#ifndef SOCKET_ERROR
#define SOCKET_ERROR -1
#endif

/* Structure used for Memory profiling & tracing... */
typedef struct{
	int8 FileName[256];
	int32 LineNo;
	int32 Size;
	void* Ptr;
}MallocStruct;

typedef struct mallocPtrLinkedList{
	MallocStruct *mallocStruct;
	struct mallocPtrLinkedList *NextPtr;
}MallocPtrLinkedList;

typedef struct sockaddr_in SOCK_INET_ADDR_STRUCT;
typedef struct sockaddr SOCK_ADDR_STRUCT;

/* Memory related functions */
#define os_new(FUN,ARG)  new FUN ARG
#define os_delete(aPtr)  if(aPtr) delete aPtr;
#define os_malloc(SIZE) _os_malloc(__FILE__,__LINE__,SIZE)

void* os_memset(IN void *aSrcPtr,IN int aVal,IN size_t aNumBytes);
void* _os_malloc(const int8 *aFileName, int32 aLineNo, IN int32 aSize);
void os_free(IN void *aPtr);
void* os_memcpy(OUT void *aDestPtr,IN const void *aSrcPtr, size_t aNumBytes);
void os_mem_trace_result();

/* String related functions */
#define os_strcpy strcpy
#define os_strcat strcat
int8* os_strncpy(OUT int8 *aDestPtr,IN const int8 *aSrcPtr, IN size_t aSize);
int32 os_strlen(int8 *aInputPtr);
int32 os_strcmp(IN const int8 *aStr1,IN const int8 *aStr2);
int8* os_strrstr(IN int8 *aStr1, IN int8 *aStr2);
int8* os_strrcasestr(IN int8 *aStr1, IN int8 *aStr2);

/* File Related Operations */
#define OS_FILE_SEEK_SET	SEEK_SET
#define OS_FILE_SEEK_CUR	SEEK_CUR
#define OS_FILE_SEEK_END	SEEK_END

#define os_fileptr FILE
#define os_fopen fopen
#define os_fclose fclose
#define os_fread fread
#define os_fwrite fwrite
#define os_fseek fseek
#define os_fsync fsync
#define os_fileno fileno
#define os_fflush fflush
#define os_ftell ftell

/*************************FUNCTIONS DEFINITIONS WHICH WILL BE IMPLEMENTED IN OS SPCIFIC FILES. *****************************/
/* Socket Related Functions. */
int32 os_set_SocketOptions(const SOCKET aSockID, SockOptions aSockOption, SockOptValue *aSockOptVal1=NULL, SockOptValue *aSockOptVal2=NULL);
ReturnStatus os_get_SocketAddress(const SOCKET aSockID, int8 *aIPAddr, int32 *aPortNo);

/* INOTIFY Related Functions. */
#define os_inotify_fd int32
#define os_inotify_watch_fd int32
#define os_inotify_mask uint32

#define OS_INOTIFY_WATCH_FLAGS (OS_INOTIFY_CREATE | \
				OS_INOTIFY_DELETE | \
				OS_INOTIFY_DELETE_SELF | \
				OS_INOTIFY_MOVE_SELF | \
				OS_INOTIFY_MOVED_FROM | \
				OS_INOTIFY_MOVED_TO | \
				OS_INOTIFY_CLOSE_WRITE)

os_inotify_fd os_inotify_init(void);
os_inotify_watch_fd os_inotify_add_watch(os_inotify_fd aFd, int8 *aPathName, os_inotify_mask aMask);
int32 os_inotify_rm_watch(os_inotify_fd aFd, os_inotify_watch_fd aWatchFd);

/* File Find related functions. */
typedef struct{
	int8 FileName[256];
	flag IsDirectory;
	flag IsSoftLink;
}os_file_find_struct;

os_dirhandle os_find_start(int8 *aDirName);
os_file_find_struct os_find_next(os_dirhandle aDirHandle);
void os_find_close(os_dirhandle aDirHandle);

/* File related Functions. */
typedef struct{
	int32	Size;
	os_time	LastAccessTime;
	os_time	LastModificationTime;
}os_file_stat_buf;

ReturnStatus os_file_stat(int8 *aFileName,os_file_stat_buf *aStatBuf);

/* Time related Functions. */
os_time_struct* os_get_time();
int8* os_ctime(os_time aTimeVal);
int32 os_get_thread_id();
int8* os_get_date();

/* Signal Handler Related Functions. */
void os_signal_register(int32 aSignal, void(*fp)(int));
/*************************FUNCTIONS DEFINITIONS WHICH WILL BE IMPLEMENTED IN OS SPCIFIC FILES. *****************************/

#endif /* OS_H_INCLUDED */

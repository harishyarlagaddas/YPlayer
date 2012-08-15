
#include "log.h"
#include "linkedlist.h"
#include "os.h"

#ifdef ANDROID
#define LOG_FILE_NAME "/sdcard/MyThought-log.txt"
#else
#define LOG_FILE_NAME "MyThought-log.txt"
#endif

static eLogLevel seLogLevel = LOG_LEVEL_TRACE;
static LOG_POINTER fpLog = NULL;
cLinkedList<int8 [64]> gLogStringsList;

void LOG_INITIALIZE()
{
	if(!fpLog){
		os_delete_file(LOG_FILE_NAME);
		fpLog = fopen(LOG_FILE_NAME,"w+");
	}
}

void LOG_ADD_STRING(const int8* aLogString)
{
	int8 tempStr[64] = {'\0'};
	if(aLogString){
		os_strncpy(tempStr,aLogString,sizeof(tempStr));
		gLogStringsList.Add(tempStr);
	}
}

LOG_POINTER GET_LOG_POINTER(const int8* aLogString)
{
	uint32 count = 0, noOfLogStrings = gLogStringsList.GetNumberOfElements();
	for(count=1; count<=noOfLogStrings; count++){
		int8 string[64] = {'\0'};
		if(FAILURE != gLogStringsList.GetElementAtIndex(string,count)){
			if(0 == os_strcmp(aLogString,string)){
				return fpLog;
			}
		}
	}
	return NULL;
}


void LOG_CONSOLE(const int8 *aFormat,...)
{
#if 1
	va_list sArgs;
	va_start(sArgs,aFormat);
#if 0
	/* Allocate the memory for the whole string + 4096 bytes. 4096 bytes is for arguments.
	  Do remember if the argment string is greater than 4096 then there is a chance for crash.
	  But we don't have any way to find out the way to know the exact length of the args string.
	  In future we do have a plans to replace Logging module. So until then please bare with it :)*/
	int32 len = os_strlen((int8*)aFormat)+8192;
	int8 *cpBuf = (int8*)os_malloc(len);
	if(cpBuf){
		vsprintf(cpBuf,aFormat,sArgs);
		printf("%s\n",cpBuf);
		os_free(cpBuf);
	}
#else
	/* Other way of implementation which is simple and error free. */
	int32 bufLen = os_strlen((int8*)aFormat) + 128;
	int8 *tempBuf = (int8*)os_malloc(bufLen);
	os_time_struct *timeStruct;
	if(NULL == tempBuf){
		return;
	}
	os_memset(tempBuf,'\0',bufLen);
	timeStruct = os_get_time();
	os_snprintf(tempBuf,bufLen,"[MyThought LOG][Time:%d:%d:%d][ThreadID:%x]",timeStruct->tm_hour,timeStruct->tm_min,timeStruct->tm_sec,os_get_thread_id());
	os_strcat(tempBuf,aFormat);
	os_strcat(tempBuf,"\n");
	vprintf(tempBuf,sArgs);
	os_free(tempBuf);
#endif
#endif
}

void Write_To_LogFile(char *cpInBuf)
{
	if(fpLog){
		fprintf(fpLog,"%s\n",cpInBuf);
		fflush(fpLog);
	}
}

void LOG_CLOSE()
{
	if(fpLog){
		fclose(fpLog);
		fpLog = NULL;
		//os_delete_file(LOG_FILE_NAME);
	}
	gLogStringsList.DestroyList();
}

void LOGF(char *aFormat,va_list aArgs)
{
#if 0
	/* Allocate the memory for the whole string + 4096 bytes. 4096 bytes is for arguments.
	  Do remember if the argment string is greater than 4096 then there is a chance for crash.
	  But we don't have any way to find out the way to know the exact length of the args string.
	  In future we do have a plans to replace Logging module. So until then please bare with it :)*/
	int8 *cpBuf = (int8*)os_malloc(os_strlen(aFormat)+8192);
	if(cpBuf){
		vsprintf(cpBuf,aFormat,aArgs);
		//fprintf(fpLog,aFormat,aArgs);
#ifdef CONSOLE
		printf("%s\n",cpBuf);
#endif
		//Write_To_LogFile(cpBuf);
		os_free(cpBuf);
	}
#else
	/* Other way of implementation which is simple and error free. */
	/* Adding the New Line to print next log statement in the new line. */
	int32 bufLen = os_strlen(aFormat) + 128;
	os_time_struct *timeStruct;
	int8 *tempBuf = (int8*)os_malloc(bufLen);
	if(NULL == tempBuf){
		return;
	}
	os_memset(tempBuf,'\0',bufLen);
	timeStruct = os_get_time();
	os_snprintf(tempBuf,bufLen,"[MyThought LOG][Time:%d:%d:%d][ThreadID:%d]",timeStruct->tm_hour,timeStruct->tm_min,timeStruct->tm_sec,os_get_thread_id());
	os_strcat(tempBuf,aFormat);
	os_strcat(tempBuf,"\n");
	vfprintf(fpLog,tempBuf,aArgs);
	//os_fsync(os_fileno(fpLog));
	os_fflush(fpLog);
#ifdef CONSOLE
	vprintf(tempBuf,aArgs);
#endif
	os_free(tempBuf);
	//os_sync();
#endif
}

void SET_LOGGING_LEVEL(eLogLevel aLogLevel)
{
	seLogLevel = aLogLevel;
}

void LOG_TRACE(const char *aFormat,...)
{
#if LOG
	if(seLogLevel >= LOG_LEVEL_TRACE){
		va_list sArgs;
		va_start(sArgs,(int8*)aFormat);
		LOGF((int8*)aFormat,sArgs);
	}
#endif
}

void LOG_DEBUG(const char *aFormat,...)
{
#if LOG
	//LOG_CONSOLE("Harish: Entered in to LOG_DEBUG for printing..");
	if(seLogLevel >= LOG_LEVEL_DEBUG){
		va_list sArgs;
		va_start(sArgs,(int8*)aFormat);
		LOGF((int8*)aFormat,sArgs);
	}
#else
	//LOG_CONSOLE("Harish: Not Entered in to LOG_DEBUG for printing..");
#endif
}

void LOG_ERROR(const char *aFormat,...)
{
#if LOG
	if(seLogLevel >= LOG_LEVEL_ERROR){
		va_list sArgs;
		va_start(sArgs,(int8*)aFormat);
		LOGF((int8*)aFormat,sArgs);
	}
#endif
}

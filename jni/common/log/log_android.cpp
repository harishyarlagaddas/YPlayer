
#include "log.h"
#define LOG_TAG "DLNAPlayerJNI"

static bool LOG = true;
static FILE *fpLog = fopen("/sdcard/.YPlayer-native.log","w+");

extern "C" {
	void InitNativeLog(){
		FILE *fp = fopen("/sdcard/.YPlayer","r");
		if(NULL != fp){
			LOG = true;
			fclose(fp);
			fpLog = fopen("/sdcard/YPlayer-native.log","w+");
		}else{
			if(fpLog){
				fclose(fpLog);
				fpLog = NULL;
			}
			LOG = true;
		}
	}

	void CloseNativeLog(){
		if(fpLog != NULL){
			fclose(fpLog);
			fpLog = NULL;
		}
	}

	void LOGDD(const int8 *aFormat,...)
	{
		if(LOG){
			va_list sArgs;
			va_start(sArgs,aFormat);
			__android_log_vprint(ANDROID_LOG_DEBUG,LOG_TAG,aFormat,sArgs);

			if(fpLog){
				vfprintf(fpLog,aFormat,sArgs);
				fprintf(fpLog,"\n");
				fflush(fpLog);
			}
		}
	}

	void LOGEE(const int8 *aFormat,...)
	{
		if(LOG){
			va_list sArgs;
			va_start(sArgs,aFormat);
			__android_log_vprint(ANDROID_LOG_ERROR,LOG_TAG,aFormat,sArgs);

			if(fpLog){
				vfprintf(fpLog,aFormat,sArgs);
				fprintf(fpLog,"\n");
				fflush(fpLog);
			}
		}
	}
}

void LOGD(const int8 *aFormat,...)
{
	if(LOG){
		va_list sArgs;
		va_start(sArgs,aFormat);
		__android_log_vprint(ANDROID_LOG_DEBUG,LOG_TAG,aFormat,sArgs);

		if(fpLog){
			vfprintf(fpLog,aFormat,sArgs);
			fprintf(fpLog,"\n");
			fflush(fpLog);
		}
	}
}

void LOGE(const int8 *aFormat,...)
{
	if(LOG){
		va_list sArgs;
		va_start(sArgs,aFormat);
		__android_log_vprint(ANDROID_LOG_ERROR,LOG_TAG,aFormat,sArgs);

		if(fpLog){
			vfprintf(fpLog,aFormat,sArgs);
			fprintf(fpLog,"\n");
			fflush(fpLog);
		}
	}
}

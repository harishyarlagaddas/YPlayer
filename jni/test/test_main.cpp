#include "test_upnp_control_point.h"

void SetLogSetting(void)
{
	int8 logStr[1024] = {'\0'}, tempStr[256] = {'\0'};
	int32 i = 0, j=0, len = 0;
	flag loggerLevelSet = false;
	os_fileptr *fp = os_fopen(LOG_INI_FILE_NAME,"r");
	if(NULL == fp){
		LOG_CONSOLE("Unable to open the file [%s]",LOG_INI_FILE_NAME);
		return;
	}

	len = os_fread(logStr,sizeof(int8),sizeof(logStr) - 1,fp);
	LOG_CONSOLE("reading from ini file of bytes [%d]",len);
	//len = os_strlen(logStr);
	while(i <= len){
		os_memset(tempStr,0,sizeof(tempStr));
		j = 0;
		while(('\n' != logStr[i] && ' ' != logStr[i] && '\t' != logStr[i]) && (i<len)){
			tempStr[j++] = logStr[i++];
		}
		i++;
		if(false == loggerLevelSet){
			int32 level = os_atoi(tempStr);
			if(0 == level){
				SET_LOGGING_LEVEL(LOG_LEVEL_ERROR);
			}else if(1 == level){
				SET_LOGGING_LEVEL(LOG_LEVEL_DEBUG);
			}else if(2 == level){
				SET_LOGGING_LEVEL(LOG_LEVEL_TRACE);
			}
			loggerLevelSet = true;
		}else{
			//LOG_CONSOLE("Adding the Logger String [%s]",tempStr);
			LOG_ADD_STRING(tempStr);
		}
	}
}

int main(int argc, char *argv[])
{
	{
		LOG_INITIALIZE();
		SetLogSetting();
		testUPNPControlPoint cpObj;
		cpObj.startTestUPNPControlPoint();
		LOG_CLOSE();
	}
	os_mem_trace_result();
	LOG_CLOSE();
	return 0;
}

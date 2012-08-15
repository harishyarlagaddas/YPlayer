#ifndef LOG_H_INCLUDED
#define LOG_H_INCLUDED

#ifdef ANDROID

#include "../include/basic_datatypes.h"
//#include "os.h"
#include "android/log.h"
#include <stdio.h>

void LOGD(const int8 *aFormat,...);
void LOGE(const int8 *aFormat,...);
#else
#include "../include/basic_datatypes.h"
#include "../../os/os.h"
#include "../include/return_codes.h"

#define LOG_POINTER FILE*
#define LOG_STRING int8 [64]

#define LOG_XML_STRING "XMLParser"
#define LOG_UPNP_DISCOVERY_STRING "UPnPDiscovery"
#define LOG_UPNP_DESCRIPTION_STRING "UPnPDescription"
#define LOB_UPNP_DATABASE_STRING "UPnPDatabase"
#define LOG_UPNP_CMS_STRING "UPnPCMS"
#define LOG_UPNP_CDS_STRING "UPnPCDS"
#define LOG_UPNP_BASE_STRING "UPnPBase"
#define LOG_UPNP_SERVER_STRING "UPnPServer"
#define LOG_UPNP_CONTROL_POINT_STRING "UPnPControlPoint"
#define LOG_SQLITE_WRAPPER_STRING "SqliteWrapper"
#define LOG_SOCKET_LOG "SocketImpl"
#define LOG_UPNP_CONTENT_STRING "UPnPContent"
#define LOG_HTTP_LISTENER_STRING "HttpListener"
#define LOG_CONTENTDIR_SCAN_STRING "ContentDirScan"
#define LOG_JPEG_PARSER_STRING "JpegParser"

typedef enum{
	LOG_LEVEL_ERROR = 0,
	LOG_LEVEL_DEBUG,
	LOG_LEVEL_TRACE
}eLogLevel;

#define LOGE LOG_ERROR
#define LOGD LOG_DEBUG

void LOG_ADD_STRING(const int8* aLogString);
LOG_POINTER GET_LOG_POINTER(const int8* aLogString);
void LOG_CONSOLE(const int8 *aFormat,...);
void Write_To_LogFile(char *cpInBuf);
void LOG_CLOSE();
void LOG_INITIALIZE();
void LOGF(char *aFormat,va_list aArgs);
void SET_LOGGING_LEVEL(eLogLevel aLogLevel);
void LOG_TRACE(const char *aFormat,...);
void LOG_DEBUG(const char *aFormat,...);
void LOG_ERROR(const char *aFormat,...);
#endif /* ANDROID */
#endif /*LOG_H_INCLUDED*/

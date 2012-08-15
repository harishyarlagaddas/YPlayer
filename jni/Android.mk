LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := dlna-cp
LOCAL_SRC_FILES := os/linux/linux_os.cpp \
		   os/os.cpp \
		   external/tinyxml/tinystr.cpp \
		   external/tinyxml/tinyxmlerror.cpp \
		   external/tinyxml/tinyxmlparser.cpp \
		   external/tinyxml/tinyxml.cpp \
		   common/log/log_android.cpp \
		   common/util/string_utils.cpp \
		   common/util/xmlcomposer.cpp \
		   common/util/XmlUtils.cpp \
		   common/socketimpl/socketimpl.cpp \
		   common/protocols/http/http_utils.cpp \
		   common/protocols/http/http_listener.cpp \
		   common/upnp/upnp_base.cpp \
  		   common/upnp/upnp_discovery.cpp \
		   common/upnp/upnp_control_point.cpp \
		   common/upnp/upnp_controlpoint_wrapper.cpp \
		   wrapper/control_point.cpp \
		   wrapper/jni_interface.c
 		   
		   

LOCAL_C_INCLUDES := \
	bionic \
	$(JNI_H_INCLUDE) \ 
	
LOCAL_LDLIBS := -L$(SYSROOT)/usr/lib -llog
LOCAL_CFLAGS += -g -fstack-check
APP_OPTIM:=debug
include $(BUILD_SHARED_LIBRARY)

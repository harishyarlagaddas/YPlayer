LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

COMMON_SRC_DIR := ../../

LOCAL_SRC_FILES:= \
	$(COMMON_SRC_DIR)log/log.cpp \
	$(COMMON_SRC_DIR)socketimpl/socketimpl.cpp \
	$(COMMON_SRC_DIR)protocols/http/http_listener.cpp \
	$(COMMON_SRC_DIR)protocols/http/http_utils.cpp \
	$(COMMON_SRC_DIR)upnp/upnp_base.cpp  \
	$(COMMON_SRC_DIR)upnp/upnp_discovery.cpp  \
	$(COMMON_SRC_DIR)upnp/upnp_description.cpp  \
	$(COMMON_SRC_DIR)upnp/upnp_cds.cpp  \
	$(COMMON_SRC_DIR)upnp/upnp_content.cpp  \
	$(COMMON_SRC_DIR)upnp/upnp_cms.cpp  \
	$(COMMON_SRC_DIR)upnp/upnp_database.cpp  \
	$(COMMON_SRC_DIR)upnp/upnp_server.cpp  \
	$(COMMON_SRC_DIR)upnp/upnp_control_point.cpp  \
	$(COMMON_SRC_DIR)upnp/contentdir_scan.cpp  \
	$(COMMON_SRC_DIR)parser/mp3parser.cpp \
	$(COMMON_SRC_DIR)parser/jpeg_parser.cpp \
	$(COMMON_SRC_DIR)util/xmlcomposer.cpp \
	$(COMMON_SRC_DIR)util/xmlparser.cpp \
	$(COMMON_SRC_DIR)util/string_utils.cpp \
	$(COMMON_SRC_DIR)database/database.cpp \
	$(COMMON_SRC_DIR)database/sqlite_wrapper.cpp

LOCAL_SHARED_LIBRARIES :=

LOCAL_CFLAGS := $(DLNA_C_FLAGS)
LOCAL_C_INCLUDES := $(DLNA_INCLUDE_DIRS)

LOCAL_MODULE:= libcommon

include $(BUILD_STATIC_LIBRARY)

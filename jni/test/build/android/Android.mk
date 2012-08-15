LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

DLNA_TEST_DIR := ../../

include $(DLNA_TOP_DIR)Android_options.mk

LOCAL_SRC_FILES := \
	$(DLNA_TEST_DIR)test_main.cpp \
	$(DLNA_TEST_DIR)test_sockimpl.cpp \
	$(DLNA_TEST_DIR)test_linkedlist.cpp \
	$(DLNA_TEST_DIR)test_upnp_server.cpp \
	$(DLNA_TEST_DIR)test_upnp_control_point.cpp \
	$(DLNA_TEST_DIR)test_xmlcomposer.cpp \
	$(DLNA_TEST_DIR)test_database.cpp \
	$(DLNA_TEST_DIR)test_xmlparser.cpp \
	$(DLNA_TEST_DIR)test_upnp_database.cpp \
	$(DLNA_TEST_DIR)test_mp3parser.cpp \
	$(DLNA_TEST_DIR)test_jpeg_parser.cpp \
	$(DLNA_TEST_DIR)test_contentdir_scan.cpp


LOCAL_MODULE := dlna

LOCAL_CFLAGS := $(DLNA_C_FLAGS)
LOCAL_C_INCLUDES := $(DLNA_INCLUDE_DIRS)

#LOCAL_PRELINK_MODULE := false

LOCAL_STATIC_LIBRARIES :=

LOCAL_SHARED_LIBRARIES := libdlna

LOCAL_LDLIBS := -lpthread

ifeq ($(TARGET_OS)-$(TARGET_SIMULATOR),linux-true)
LOCAL_LDLIBS += -ldl
endif
ifneq ($(TARGET_SIMULATOR),true)
LOCAL_SHARED_LIBRARIES += libdl
endif

include $(BUILD_EXECUTABLE)

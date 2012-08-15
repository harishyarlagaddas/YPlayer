LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

SRC_DIR := $(LOCAL_PAHT)/../../

LOCAL_SRC_FILES:= \
    $(SRC_DIR)linux/linux_os.cpp \
    $(SRC_DIR)os.cpp

LOCAL_SHARED_LIBRARIES :=

LOCAL_MODULE:= libos

LOCAL_CFLAGS := $(DLNA_C_FLAGS)
LOCAL_C_INCLUDES := $(DLNA_INCLUDE_DIRS)

include $(BUILD_STATIC_LIBRARY)

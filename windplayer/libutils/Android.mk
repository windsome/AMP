LOCAL_PATH:= $(call my-dir)

common_SRC_FILES := \
  util_log.cpp \
  util_lock.cpp \
  util_thread.cpp \
  util_time.cpp \
  util_timer.cpp \
  util_uri.cpp \
  util_network.cpp \
  util_file.cpp \
  util_fsinfo.cpp \
  util_folder.cpp \
  util_eventloop.cpp \
  util_md5sum.cpp \
  util_crc32.cpp \
  util_format.cpp \
  util_ringbuffer.cpp \
  util_argument.cpp \
  util_cmdline.cpp


common_CFLAGS := -W -g -DPLATFORM_ANDROID

common_C_INCLUDES +=\
    $(LOCAL_PATH) \
    $(LOCAL_PATH)/../ustl-1.0

include $(CLEAR_VARS)
LOCAL_SRC_FILES := $(common_SRC_FILES)
LOCAL_CFLAGS += $(common_CFLAGS)
LOCAL_C_INCLUDES += $(common_C_INCLUDES)
LOCAL_SHARED_LIBRARIES := libutils libstdc++
LOCAL_STATIC_LIBRARIES := libustl
LOCAL_PRELINK_MODULE := false
LOCAL_MODULE := libwindutils 
include $(BUILD_STATIC_LIBRARY)
#include $(BUILD_SHARED_LIBRARY)


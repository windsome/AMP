LOCAL_PATH:= $(call my-dir)
common_CFLAGS := -DPLATFORM_ANDROID=1 -D__STDC_CONSTANT_MACROS=1
ffmpeg_LIBRARIES := libavformat libavcodec libavutil
#ffmpeg_LIBRARIES := libavutil libavformat libavcodec libswscale 


common_SRC_FILES :=\
    packetList.cpp \
    decoder.cpp \
    decoder_impl.cpp \
    masterclock.cpp \
    output.cpp \
    videoout.cpp \
    audioout.cpp \
	windplayer.cpp

common_C_INCLUDES = $(LOCAL_PATH) \
					$(LOCAL_PATH)/../../ustl-1.0 \
					$(LOCAL_PATH)/../../libutils \
					external/ffmpeg-android


#########################
# Build the libwindplayer library
include $(CLEAR_VARS)
LOCAL_SRC_FILES :=$(common_SRC_FILES)
LOCAL_C_INCLUDES := $(common_C_INCLUDES)
LOCAL_CFLAGS += $(common_CFLAGS)
LOCAL_SHARED_LIBRARIES := libstdc++ libutils
#LOCAL_STATIC_LIBRARIES := libwindutils libustl $(ffmpeg_LIBRARIES)
LOCAL_PRELINK_MODULE := false
LOCAL_MODULE:= libwindplayer
include $(BUILD_STATIC_LIBRARY)
#include $(BUILD_SHARED_LIBRARY)


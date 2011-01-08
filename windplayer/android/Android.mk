LOCAL_PATH:= $(call my-dir)
common_CFLAGS := -DPLATFORM_ANDROID=1 -D__STDC_CONSTANT_MACROS=1
ffmpeg_LIBRARIES := libavformat libavcodec libswscale libavutil 

common_SRC_FILES := android_audio.cpp \
					android_video.cpp \
					android_outer.cpp \
					android_windplayer.cpp

common_C_INCLUDES += $(LOCAL_PATH) \
					$(LOCAL_PATH)/../lib \
					$(LOCAL_PATH)/../../ustl-1.0 \
					$(LOCAL_PATH)/../../libutils \
					include/graphics \
					include/binder \
					external/ffmpeg-android

#########################
# Build the libwindplayer2 library
include $(CLEAR_VARS)
LOCAL_SRC_FILES := $(common_SRC_FILES)
LOCAL_C_INCLUDES += $(common_C_INCLUDES)
LOCAL_CFLAGS += $(common_CFLAGS)
LOCAL_PRELINK_MODULE := false
LOCAL_SHARED_LIBRARIES := libutils libcutils libui libandroid_runtime liblog libbinder libmedia libstdc++ libz
#LOCAL_STATIC_LIBRARIES := libwindplayer libwindutils libustl $(ffmpeg_LIBRARIES)
LOCAL_MODULE:= libwindplayer2
include $(BUILD_STATIC_LIBRARY)
#include $(BUILD_SHARED_LIBRARY)


#########################
# Build the test wplayer.
include $(CLEAR_VARS)
LOCAL_SRC_FILES:= main.cpp
LOCAL_C_INCLUDES += $(common_C_INCLUDES)
LOCAL_CFLAGS += $(common_CFLAGS)
LOCAL_SHARED_LIBRARIES := libutils libcutils libui libandroid_runtime liblog libbinder libmedia libstdc++ libz
LOCAL_STATIC_LIBRARIES := libwindplayer2 libwindplayer libwindutils libustl $(ffmpeg_LIBRARIES) 
#LOCAL_SHARED_LIBRARIES := libwindplayer2
LOCAL_MODULE:= wplayer
include $(BUILD_EXECUTABLE)


LOCAL_PATH:= $(call my-dir)
common_CFLAGS := -DPLATFORM_ANDROID=1 -D__STDC_CONSTANT_MACROS=1

common_SRC_FILES := wind_media_WindPlayer.cpp

common_C_INCLUDES += $(LOCAL_PATH) \
					$(LOCAL_PATH)/../android \
					$(LOCAL_PATH)/../lib \
					$(LOCAL_PATH)/../../ustl-1.0 \
					$(LOCAL_PATH)/../../libutils \
					include/graphics \
					include/binder \
					external/ffmpeg-android \
					frameworks/base/core/jni \
					$(JNI_H_INCLUDE) \


ffmpeg_LIBRARIES := libavformat libavcodec libswscale libavutil 

#########################
# Build the libwindplayer_jni library
include $(CLEAR_VARS)
LOCAL_SRC_FILES:= $(common_SRC_FILES)
LOCAL_C_INCLUDES += $(common_C_INCLUDES)
LOCAL_CFLAGS += $(common_CFLAGS)
LOCAL_PRELINK_MODULE := false
LOCAL_SHARED_LIBRARIES := libutils libcutils libui libandroid_runtime liblog libbinder libmedia libstdc++ libnativehelper libz
LOCAL_STATIC_LIBRARIES := libwindplayer2 libwindplayer libwindutils libustl $(ffmpeg_LIBRARIES) 
LOCAL_MODULE:= libwindplayer2_jni
include $(BUILD_SHARED_LIBRARY)


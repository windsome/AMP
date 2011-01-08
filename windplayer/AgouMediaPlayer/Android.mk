LOCAL_PATH:= $(call my-dir)

# Build activity

include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := samples
#LOCAL_SRC_FILES := $(call all-subdir-java-files)
LOCAL_SRC_FILES := $(call all-java-files-under, src)
LOCAL_PACKAGE_NAME := AgouMediaPlayer
LOCAL_JNI_SHARED_LIBRARIES := libwindplayer2_jni
#LOCAL_LDLIBS := libcurl libupnp
include $(BUILD_PACKAGE)


LOCAL_PATH:= $(call my-dir)

# ---------------------------------------
# First project
# 
# Build DRM2 core library
#
# Output: libdrm2.so
# ---------------------------------------

common_SRC_FILES := \
    bktrace.cpp \
    memblock.cpp \
    ofstream.cpp \
    ualgobase.cpp \
    unew.cpp \
    cmemlink.cpp \
    memlink.cpp \
    sistream.cpp \
    ubitset.cpp \
    ustdxept.cpp \
    fstream.cpp \
    mistream.cpp \
    sostream.cpp \
    uexception.cpp \
    ustring.cpp \


common_CFLAGS := -W -g -DPLATFORM_ANDROID -DANDROID

common_C_INCLUDES +=\
    $(LOCAL_PATH) 

############shared lib#######
include $(CLEAR_VARS)
LOCAL_CFLAGS += -fstrict-aliasing -fomit-frame-pointer $(common_CFLAGS)
LOCAL_SRC_FILES := $(common_SRC_FILES)
#LOCAL_C_INCLUDES := $(common_C_INCLUDES)
#LOCAL_SHARED_LIBRARIES := libstdc++
#LOCAL_PRELINK_MODULE := false
LOCAL_ARM_MODE := arm
LOCAL_MODULE := libustl
#include $(BUILD_SHARED_LIBRARY)
include $(BUILD_STATIC_LIBRARY)

############static lib#######
#include $(CLEAR_VARS)
#LOCAL_CFLAGS += -fstrict-aliasing -fomit-frame-pointer
#LOCAL_CFLAGS += $(common_CFLAGS)
#LOCAL_SRC_FILES := $(common_SRC_FILES)
#LOCAL_C_INCLUDES := $(common_C_INCLUDES)
##LOCAL_SHARED_LIBRARIES := libstdc++
#LOCAL_PRELINK_MODULE := false
#LOCAL_ARM_MODE := arm
#LOCAL_MODULE := libustl
#include $(BUILD_STATIC_LIBRARY)



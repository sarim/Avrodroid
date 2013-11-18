LOCAL_PATH := $(call my-dir)
MY_LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE    := avrov8
LOCAL_C_INCLUDES := $(LOCAL_PATH)/external/v8/include

LOCAL_SRC_FILES := avrov8.cpp
LOCAL_STATIC_LIBRARIES := v8
LOCAL_LDLIBS := -llog

include $(BUILD_SHARED_LIBRARY)
ENABLE_V8_SNAPSHOT = true

include $(MY_LOCAL_PATH)/Helper.mk

include $(MY_LOCAL_PATH)/external/v8/Android.libv8.mk

include $(CLEAR_VARS)

include $(MY_LOCAL_PATH)/external/latinime/Android.mk

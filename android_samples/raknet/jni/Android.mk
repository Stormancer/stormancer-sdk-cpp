LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := raknet
MY_PREFIX := $(LOCAL_PATH)/../src/
MY_SOURCES := $(wildcard $(MY_PREFIX)*.cpp)
LOCAL_SRC_FILES += $(MY_SOURCES:$(MY_PREFIX)%=../src/%)

include $(BUILD_SHARED_LIBRARY)

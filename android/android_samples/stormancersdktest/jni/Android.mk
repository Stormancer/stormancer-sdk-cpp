LOCAL_PATH := $(call my-dir)

# RAKNET

include $(CLEAR_VARS)
LOCAL_MODULE := raknet
LOCAL_SRC_FILES := ../../raknet/libs/$(TARGET_ARCH_ABI)/libraknet.so
LOCAL_EXPORT_C_INCLUDES += $(LOCAL_PATH)/../../../../RakNet/Source
include $(PREBUILT_SHARED_LIBRARY)

# Stormander SDK CPP

include $(CLEAR_VARS)
LOCAL_MODULE := stormancersdkcpp
LOCAL_SRC_FILES := ../../stormancersdkcpp/libs/$(TARGET_ARCH_ABI)/libstormancersdkcpp.so
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/../../../../src
#LOCAL_EXPORT_C_INCLUDES += $(LOCAL_PATH)/../../../../src/Dto
#LOCAL_EXPORT_C_INCLUDES += $(LOCAL_PATH)/../../../../src/Plugins
#LOCAL_EXPORT_C_INCLUDES += $(LOCAL_PATH)/../../../../src/Transports
LOCAL_EXPORT_C_INCLUDES += $(LOCAL_PATH)/../../../../msgpack-c/include
LOCAL_EXPORT_C_INCLUDES += $(LOCAL_PATH)/../../../../casablanca/Release/include
LOCAL_EXPORT_C_INCLUDES += $(LOCAL_PATH)/../../../../casablanca/Build_android/build/Boost-for-Android/boost_1_55_0
LOCAL_EXPORT_C_INCLUDES += $(LOCAL_PATH)/../../../../rxcpp/Rx/v2/src/rxcpp
include $(PREBUILT_SHARED_LIBRARY)

# wrapper info

include $(CLEAR_VARS)
LOCAL_MODULE := stormancersdktest
LOCAL_SHARED_LIBRARIES := raknet
LOCAL_SHARED_LIBRARIES += stormancersdkcpp
LOCAL_C_INCLUDES += ../src
LOCAL_SRC_FILES := ../src/main.cpp ../src/AndroidLogger.cpp
LOCAL_LDLIBS += -lm -llog
LOCAL_CPP_EXTENSION += .cpp .cxx .cc .c
include $(BUILD_SHARED_LIBRARY)

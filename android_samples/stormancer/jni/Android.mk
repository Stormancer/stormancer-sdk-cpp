LOCAL_PATH := $(call my-dir)

# RAKNET

include $(CLEAR_VARS)
LOCAL_MODULE := raknet
LOCAL_SRC_FILES := ../raknet/libs/$(TARGET_ARCH_ABI)/libraknet.so
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/../raknet/src
include $(PREBUILT_SHARED_LIBRARY)

# CPPREST

include $(CLEAR_VARS)
LOCAL_MODULE := cpprest
LOCAL_SRC_FILES := ../casablanca/Build_android/build/build.armv7.debug/Binaries/libcpprest.a
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/../casablanca/Release/include
include $(PREBUILT_STATIC_LIBRARY)

# BOOST

include $(CLEAR_VARS)
LOCAL_MODULE := libboost_atomic
LOCAL_SRC_FILES := ../casablanca/Build_android/build/Boost-for-Android/build/lib/libboost_atomic-clang-mt-d-1_55.a
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/../casablanca/Build_android/build/Boost-for-Android/boost_1_55_0
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libboost_chrono
LOCAL_SRC_FILES := ../casablanca/Build_android/build/Boost-for-Android/build/lib/libboost_chrono-clang-mt-d-1_55.a
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/../casablanca/Build_android/build/Boost-for-Android/boost_1_55_0
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libboost_date_time
LOCAL_SRC_FILES := ../casablanca/Build_android/build/Boost-for-Android/build/lib/libboost_date_time-clang-mt-d-1_55.a
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/../casablanca/Build_android/build/Boost-for-Android/boost_1_55_0
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libboost_filesystem
LOCAL_SRC_FILES := ../casablanca/Build_android/build/Boost-for-Android/build/lib/libboost_filesystem-clang-mt-d-1_55.a
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/../casablanca/Build_android/build/Boost-for-Android/boost_1_55_0
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libboost_locale
LOCAL_SRC_FILES := ../casablanca/Build_android/build/Boost-for-Android/build/lib/libboost_locale-clang-mt-d-1_55.a
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/../casablanca/Build_android/build/Boost-for-Android/boost_1_55_0
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libboost_random
LOCAL_SRC_FILES := ../casablanca/Build_android/build/Boost-for-Android/build/lib/libboost_random-clang-mt-d-1_55.a
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/../casablanca/Build_android/build/Boost-for-Android/boost_1_55_0
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libboost_system
LOCAL_SRC_FILES := ../casablanca/Build_android/build/Boost-for-Android/build/lib/libboost_system-clang-mt-d-1_55.a
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/../casablanca/Build_android/build/Boost-for-Android/boost_1_55_0
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libboost_thread
LOCAL_SRC_FILES := ../casablanca/Build_android/build/Boost-for-Android/build/lib/libboost_thread-clang-mt-d-1_55.a
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/../casablanca/Build_android/build/Boost-for-Android/boost_1_55_0
include $(PREBUILT_STATIC_LIBRARY)

# OPENSSL

include $(CLEAR_VARS)
LOCAL_MODULE := libssl
LOCAL_SRC_FILES := ../casablanca/Build_android/build/openssl/armeabi-v7a/lib/libssl.a
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/../casablanca/Build_android/build/openssl/armeabi-v7a/include/openssl
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libcrypto
LOCAL_SRC_FILES := ../casablanca/Build_android/build/openssl/armeabi-v7a/lib/libcrypto.a
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/../casablanca/Build_android/build/openssl/armeabi-v7a/include/openssl
include $(PREBUILT_STATIC_LIBRARY)








# STORMANCERSDK

include $(CLEAR_VARS)
LOCAL_MODULE := stormancer-sdk-android

# include rxcpp
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../rxcpp/Rx/v2/src/rxcpp

# msgpack
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../src/msgpack-c/include

# include stormancer sdk
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../src/src

LOCAL_SHARED_LIBRARIES := raknet

LOCAL_STATIC_LIBRARIES := cpprest
LOCAL_STATIC_LIBRARIES += libboost_atomic
LOCAL_STATIC_LIBRARIES += libboost_chrono
LOCAL_STATIC_LIBRARIES += libboost_date_time
LOCAL_STATIC_LIBRARIES += libboost_filesystem
LOCAL_STATIC_LIBRARIES += libboost_locale
LOCAL_STATIC_LIBRARIES += libboost_random
LOCAL_STATIC_LIBRARIES += libboost_system
LOCAL_STATIC_LIBRARIES += libboost_thread
LOCAL_STATIC_LIBRARIES += libssl
LOCAL_STATIC_LIBRARIES += libcrypto

FILE_LIST := $(wildcard $(LOCAL_PATH)/../src/src/*.cpp)
FILE_LIST += $(wildcard $(LOCAL_PATH)/../src/src/Dto/*.cpp)
#FILE_LIST += $(wildcard $(LOCAL_PATH)/../src/src/Plugins/*.cpp)
FILE_LIST += $(wildcard $(LOCAL_PATH)/../src/src/Transports/*.cpp)
LOCAL_SRC_FILES := $(FILE_LIST:$(LOCAL_PATH)/%=%)
#LOCAL_SRC_FILES := main.cpp

LOCAL_LDLIBS += -lm -llog
LOCAL_CPP_EXTENSION += .cpp .cxx .cc .c

include $(BUILD_SHARED_LIBRARY)

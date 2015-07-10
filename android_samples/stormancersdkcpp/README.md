## How to build stormancersdkcpp for android

### Prerequisites

Download and install cygwin64 and check this packages:  
- devel / cmake
- devel / gcc-core
- devel / git
- devel / make
- devel / openssl-devel
- libs / libboost-devel
- perl / perl
- web / wget

### Build

Unzip *libcpprest.zip* in the same directory. Location: *casablanca/Build_android/build/build.armv7.debug/Binaries/libcpprest.zip*  

Run cygwin64 and type:
```
cd /cygdrive/c/Users/<username>/stormancer-sdk-cpp/android_samples/stormancersdkcpp/
/cygdrive/c/cygwin64/home/antla/android-ndk-r10e/ndk-build
```

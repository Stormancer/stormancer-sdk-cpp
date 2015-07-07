## How to build stormancer-sdk-cpp for android

### Links
- https://casablanca.codeplex.com/wikipage?title=Setup%20and%20Build%20on%20Android&referringTitle=Use%20on%20Android
- https://casablanca.codeplex.com/wikipage?title=Use%20on%20Android&referringTitle=Setup%20and%20Build%20on%20Android
- http://dl.google.com/android/ndk/android-ndk32-r10-linux-x86_64.tar.bz2
- http://dl.google.com/android/ndk/android-ndk32-r10-windows-x86_64.zip

### Preparation

Download and install cygwin64 and check this packages:  
- devel -> cmake
- devel -> gcc-core
- devel -> git
- devel -> make
- devel -> openssl-devel
- libs -> libboost-devel
- perl -> perl
- web -> wget

Run a windows cmd and type:
```
cd c:\cygwin64\home\username\android-ndk-r10\samples\stormancer\
mklink /D src C:\Users\username\DEV\stormancer-sdk-cpp\
mklink /D rxcpp C:\Users\username\DEV\RxCpp
mklink /D casablanca C:\Users\username\DEV\casablanca
mklink /D raknet C:\cygwin64\home\username\android-ndk-r10\samples\raknet
```

### Build

Run cygwin64 and type:
```
cd android-ndk-r10/samples/stormancer
../../ndk-build
```

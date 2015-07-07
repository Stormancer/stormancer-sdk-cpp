## How to build raknet for android

### Links
- https://github.com/OculusVR/RakNet

If you really want to rebuild raknet. Read the **Preparation** part here before continuing:  
https://github.com/Stormancer/stormancer-sdk-cpp/blob/master/android_samples/stormancer/README.md

Run a windows cmd and type:  
```
cd c:\cygwin64\home\username\android-ndk-r10\samples\raknet\
mklink /D src C:\Users\username\stormancer-sdk-cpp\RakNet\Source
```

Run cygwin64 and type:  
```
cd android-ndk-r10/samples/raknet
../../ndk-build
```

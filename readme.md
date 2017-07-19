# Stormancer SDK C++


## Introduction

A NuGet Package is available. Use it for a simpler setup.  


## Unreal Engine 4

You can download the UE4 plugin in the Releases section of the github repository.  
https://github.com/Stormancer/stormancer-sdk-cpp/releases


## Documentation

http://stormancer.github.io/stormancer-sdk-cpp/documentation/html/annotated.html


## Configure your project

We will suppose you are using Visual Studio. The library uses C++11 features, so requires at least VS 2013. However, we recommand you to use VS 2015 if possible. Deployment bugs regarding the runtime version may subsist on VS 2013.  

*Don't forget to adjust the paths. We assume your project is in a directory beside your local stormancer-sdk-cpp repository.*  

- Open and build the stormancer-sdk-cpp solution. This will build CppRestSDK, Raknet and the library itself as a single Windows static lib.  
*We recently changed our build process and we add to temporarily remove dynamic library outputs, but please ask if you need other kind of build outputs.*
- Create / open your project.  
- Open the **project properties**  
- Select **All Configurations** and **All Platforms** on top of the property window.  
- Change the **Additional include directories**  
*(Configuration Properties > C/C++ > General > Additional include directories)*  
```
$(SolutionDir)..\stormancer-sdk-cpp\output\include\
```
- Change the **Additional library directories**  
*(Configuration Properties > Linker > General > Additional library directories)*  
```
$(SolutionDir)..\stormancer-sdk-cpp\output\libs\
```
- Change the **Additional dependencies**  
*(Configuration Properties > Linker > Input > Additional dependencies)*  
```
Stormancer$(PlatformToolsetVersion)_$(Configuration)_$(Platform).lib
```
- Build your project.  

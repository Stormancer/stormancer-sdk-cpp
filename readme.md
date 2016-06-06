# Stormancer SDK C++

## Documentation

http://stormancer.github.io/stormancer-sdk-cpp/documentation/html/annotated.html

## prerequisites

### Bundled libraries
- RakNet  
- MsgPack-c  
- RxCpp  
- CppRestSDK *(casablanca)*  

## Configure the samples
For each sample, you have to do this:  
- Change the **Environment path**  
*(Project properties > Configuration Properties > Debugging > Environment)*  
```
PATH=$(SolutionDir)..\stormancer-sdk-cpp\cpprestsdk\Binaries\$(Platform)\$(Configuration);$(SolutionDir)..\stormancer-sdk-cpp\RakNet\Lib\DLL\Lib;$(SolutionDir)..\stormancer-sdk-cpp\bin;%PATH%
```

## Configure a project

*Don't forget to adjust the paths. We assume your project is in a directory beside the stormancer-sdk-cpp.*  
- Build the SDK and the samples before continuing.  
*You only need to restore the NuGet packages and set the envrionment path of the sdk and the sample projects to achieve this.*  
- Create / open your project.  
- Open the **project properties**  
- Select **All Configurations** and **All Platforms** *(or your target platform)* on top of the property window.  
- Change the **Environment path**  
*(Configuration Properties > Debugging > Environment)*  
```
PATH=$(SolutionDir)..\stormancer-sdk-cpp\cpprestsdk\Binaries\$(Platform)\$(Configuration);$(SolutionDir)..\stormancer-sdk-cpp\RakNet\Lib\DLL\Lib;$(SolutionDir)..\stormancer-sdk-cpp\bin;%PATH%
```
- Change the **Additional include directories**  
*(Configuration Properties > C/C++ > General > Additional include directories)*  
```
$(SolutionDir)..\stormancer-sdk-cpp\src
$(SolutionDir)..\stormancer-sdk-cpp\cpprestsdk\Release\include
$(SolutionDir)..\stormancer-sdk-cpp\msgpack-c\include
$(SolutionDir)..\stormancer-sdk-cpp\RakNet\Source
$(SolutionDir)..\stormancer-sdk-cpp\rxcpp\Rx\v2\src\rxcpp
```
- Change the **Additional library directories**  
*(Configuration Properties > Linker > General > Additional library directories)*  
```
$(SolutionDir)..\stormancer-sdk-cpp\bin
$(SolutionDir)..\stormancer-sdk-cpp\RakNet\Lib\DLL\Lib
$(SolutionDir)..\stormancer-sdk-cpp\cpprestsdk\Binaries\$(Platform)\$(Configuration)
```
- Change the **Additional dependencies**  
*(Configuration Properties > Linker > Input > Additional dependencies)*  
```
Stormancer$(PlatformToolsetVersion)_$(Configuration)_$(Platform).lib
RakNet$(PlatformToolsetVersion)_$(Configuration)_$(Platform).lib
```
- Build the project.  


## Configure an **Unreal Engine 4** Visual Studio project

You can now download the UE4 plugin in the Releases section of the github repository  
https://github.com/Stormancer/stormancer-sdk-cpp/releases


## Generate the docs

- switch to master branch  
- Get the last version of the stormancer sdk  
- Get doxygen here : www.doxygen.org  
- Install and open doxygen  
- Open the configuration file *./documentation/Doxyfile* with doxygen  
- Generate the documentation  
- Merge the master branch in the gh-pages branch  
- Push the master and gh-pages branches in the github repositotry  

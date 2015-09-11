# Stormancer SDK C++

## Documentation

http://stormancer.github.io/stormancer-sdk-cpp/documentation/html/annotated.html

## prerequisites

### Bundled libraries
- RakNet *(pre-built for windows 32/64 bits)*  
- MsgPack  

## Configure the samples
For each sample, you have to do this:  
- Change the **Environment path**  
*(Project properties > Configuration Properties > Debugging > Environment)*  
```
PATH=$(SolutionDir)RakNet\Lib\DLL\Lib;$(SolutionDir)bin;%PATH%
```

## Configure a project  
*Don't forget to adjust the paths. We assume your project is in a directory beside the stormancer-sdk-cpp.*  
- Build the SDK and the samples before continuing.  
*You only need to restore the NuGet packages and set the envrionment path of the sdk and the sample projects to achieve this.*  
- Create / open your project.  
- Install this NuGet packages:  
    - C++ REST SDK (by searching casablanca or cpprestsdk)  
    - Reactive Extensions for C++ (by searching rxcpp)  
- Open the **project properties**  
- Select **All Configurations** and **All Platforms** *(or your target platform)* on top of the property window.  
- Change the **Environment path**  
*(Configuration Properties > Debugging > Environment)*  
```
PATH=$(SolutionDir)..\stormancer-sdk-cpp\RakNet\Lib\DLL\Lib;$(SolutionDir)..\stormancer-sdk-cpp\bin;%PATH%
```
- Change the **Additional include directories**  
*(Configuration Properties > C/C++ > General > Additional include directories)*  
```
$(SolutionDir)..\stormancer-sdk-cpp\src
$(SolutionDir)..\stormancer-sdk-cpp\msgpack-c\include
$(SolutionDir)..\stormancer-sdk-cpp\RakNet\Source
```
- Change the **Additional library directories**  
*(Configuration Properties > Linker > General > Additional library directories)*  
```
$(SolutionDir)..\stormancer-sdk-cpp\RakNet\Lib\DLL\Lib
$(SolutionDir)..\stormancer-sdk-cpp\bin
```
- Change the **Additional dependencies**  
*(Configuration Properties > Linker > Input > Additional dependencies)*  
```
DLL_vc9_DLL_$(Configuration)_$(Platform).lib
stormancer-sdk-cpp_$(Platform)_$(Configuration).lib
```
- Build the project.  

## Generate the docs

- switch to master branch  
- Get the last version of the stormancer sdk  
- Get doxygen here : www.doxygen.org  
- Install and open doxygen  
- Open the configuration file *./documentation/Doxyfile* with doxygen  
- Generate the documentation  
- Merge the master branch in the gh-pages branch  
- Push the master and gh-pages branches in the github repositotry  

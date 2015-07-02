# Stormancer SDK C++

## Documentation

http://stormancer.github.io/stormancer-sdk-cpp/documentation/html/annotated.html

## prerequisites

### Bundled libraries
- RakNet *(pre-built for windows 32/64 bits)*  
- MsgPack  

### NuGet packages to install
- C++ REST SDK (or cpprestsdk) (by searching casablanca)  
- Reactive Extensions for C++ (or rxcpp)  

## Configure the samples
- Change the **Environment path**  
*(Configuration Properties > Debugging > Environment)*  
```
PATH=$(SolutionDir)RakNet\Lib\DLL\Lib;$(SolutionDir)bin;%PATH%  
```

## Configure a project

- Build the SDK.  
- Open the **project properties**  
- Select **All Configurations** and **All Platforms** *(or your target platform)* on top of the property window.  
- Change the **Environment path**  
*(Configuration Properties > Debugging > Environment)*  
```
PATH=$(SolutionDir)..\..\stormancer-sdk-cpp\RakNet\Lib\DLL\Lib;$(SolutionDir)..\..\stormancer-sdk-cpp\bin;%PATH%  
```
- Change the **Additional include directories**  
*(Configuration Properties > C/C++ > General > Additional include directories)*  
```
$(SolutionDir)..\..\stormancer-sdk-cpp\include  
$(SolutionDir)..\..\stormancer-sdk-cpp\MsgPack  
$(SolutionDir)..\..\stormancer-sdk-cpp\RakNet\Source  
```
- Change the **Additional library directories**  
*(Configuration Properties > Linker > General > Additional library directories)*  
```
$(SolutionDir)..\..\stormancer-sdk-cpp\RakNet\Lib\DLL\Lib  
$(SolutionDir)..\..\stormancer-sdk-cpp\bin  
```
- Change the **Additional dependencies**  
*(Configuration Properties > Linker > Input > Additional dependencies)*  
```
DLL_vc9_DLL_$(Configuration)_$(Platform).lib  
ws2_32.lib  
stormancer-sdk-cpp_Win32_Debug.lib  
```
- If the project is in the Stormancer SDK solution. Add a reference.
- If the project has its own solution. Reference manually the SDK to the project.
- Add the *MsgPack/MsgPack.cpp* file of the SDK to the project.  
- Build the project.  

*Add the NuGet packages mentioned above if Visual Studio don't get it automatically when building the project*.  

## Generate the docs

- switch to master branch  
- Get the last version of the stormancer sdk  
- Get doxygen here : www.doxygen.org  
- Install and open doxygen  
- Open the configuration file in ./documentation/Doxyfile  
- Generate the documentation  
- Merge the master branch in the gh-pages branch  
- Push the master and gh-pages branches in the github repositotry  

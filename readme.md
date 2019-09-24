# Stormancer SDK C++

## Building the C++ Client Library

Requirements
------------

- Microsoft Visual Studio 2017
- Visual C++ v141 build tools
- .Net framework 4.7.1 SDK (needed for our build process automation tools)

How to Build
------------

There is a Visual Studio solution for each supported platform. Note that the open source version of the library currently only supports Windows and Android. If you need support for other platforms, please contact us.

Simply open and build the Visual Studio solution for your platform and configuration of choice.

The build products are located in the `output` directory by default. `output/include` contains the header files, and `output/libs` the static libraries.

Plugins
*******

Plugins can be added to the library on-demand.
To do so, create a file named `plugins.json` under `src/clients/cpp`.
It should have the following format :

	{
		"plugins": [
			{ "name": "<name of a plugin>" },
			...
		]
	}

Note that plugin dependencies are not automatically handled. So for instance, if you want to include the `Authentication` plugin, you also need to explicitly include the `Core` plugin, which `Authentication` depends on.

For instance, if you need plugins `Authentication` and `GameFinder`, you would write the following `plugins.json`:

	{
		"plugins": [
			{ "name": "Core" },
			{ "name": "Authentication" },
			{ "name": "GameFinder" }
		]
	}

Note that the order in which the plugins are specified doesn't matter.

## Using the Library in your Project

Using the library should be as simple as adding the client static library (`Stormancer<platform toolset>_<configuration>_<platform>.lib`) to your linker inputs,
and adding the `include` directory (the one that was produced by the build) to your compiler's list of additional include directories.

Here is a more detailed step-by-step guide for Visual Studio:

### Visual Studio Quick Start Guide

- Create / open your project.
- Create a new folder named `stormancer` next to your VS project file.
- Copy the contents of the `output` folder into the `stormancer` folder.
- Open the **project properties**  for your project.
- Select **All Configurations** and **All Platforms** on top of the property window.  
- Add the following line to your **Additional include directories**  
*(Configuration Properties > C/C++ > General > Additional include directories)*  
```
stormancer\include\
```
- Add the following line to your **Additional library directories**  
*(Configuration Properties > Linker > General > Additional library directories)*  
```
stormancer\libs\
```
- Add the following line to your **Additional dependencies**  
*(Configuration Properties > Linker > Input > Additional dependencies)*  
```
Stormancer$(PlatformToolsetVersion)_$(Configuration)_$(Platform).lib
```
- Build your project.  

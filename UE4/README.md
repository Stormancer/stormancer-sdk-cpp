Stormancer Plugin for Unreal Engine 4
=====================================

Installation
------------

The Stormancer Unreal plugin embeds the Stormancer library.
You can either use a pre-built Stormancer release, or build Stormancer yourself and use the resulting library in the plugin.

- Inside your UE4 project's directory, create a directory named `Plugins` if it doesn't already exist.

### Using a pre-built Stormancer release

- Copy the `stormancerPlugin` folder into your UE project's Plugins directory.

### Using a self-made source build

- Build Stormancer for the desired platforms and configurations using Visual Studio.

- Run `BuildUEPlugin.bat`.

- Copy the `stormancerPlugin` folder in `UE4/source_build` into your UE project's Plugins directory.

### Steps common to both methods

- Inside your project's build configuration file (`MyProject/Source/MyProject/MyProject.Build.cs`), add a dependency to the `StormancerPlugin` module.
Also, as Stormancer needs exceptions and RTTI, you need to add these two lines to the build file's constructor:
```
bEnableExceptions = true;
bUseRTTI = true;
```

- The plugin will be built and added to your project next time you load the project in the Unreal Editor, or the next time you build your game.
If the project was already open, you will need to reload it.

Usage
-----

Include `"IStormancerPlugin.h"` in the source files where you want to use Stormancer.

The `CreateClient()` and `GetClient()` methods allow you to instantiate and retrieve a single `Stormancer::Client`
that will persist as long as the plugin is loaded (which means as long as your game is running),
regardless of how many references you keep to it.

You can then proceed to use this Client with the regular Stormancer API.

**Note**: You must not issue blocking calls inside `pplx::task` bodies, as these are run on a single thread.
Especially, both launching and waiting for a task to complete from within another task will result in a deadlock.
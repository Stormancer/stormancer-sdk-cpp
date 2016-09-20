// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "StormancerPluginPrivatePCH.h"

extern "C" {
	#include "extern/extern.h"
}

class FStormancerPlugin : public IStormancerPlugin
{
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};

IMPLEMENT_MODULE( FStormancerPlugin, StormancerPlugin )



void FTestPlugin::StartupModule()
{
	// This code will execute after your module is loaded into memory (but after global variables are initialized, of course.)
	auto value = new npp::DummyDynamicLoader();
	delete value;
  auto v2 = rs_trigger(100);
}


void FTestPlugin::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}
// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "StormancerPluginPrivatePCH.h"

DEFINE_LOG_CATEGORY(StormancerLog);

class FStormancerPlugin : public IStormancerPlugin
{
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	void DestroyClient();

public:
	std::shared_ptr<Stormancer::Client> CreateClient(std::string endpoint, std::string account, std::string application) override;
	std::shared_ptr<Stormancer::Client> GetClient() override;

private:
	std::shared_ptr<Stormancer::Client> _Client;

#if WITH_EDITOR
	FDelegateHandle _PIECleanup;
#endif
};

IMPLEMENT_MODULE( FStormancerPlugin, StormancerPlugin )



void FStormancerPlugin::StartupModule()
{
	// This code will execute after your module is loaded into memory (but after global variables are initialized, of course.)
	UE_LOG(StormancerLog, Display, TEXT("Starting Stormancer"));

#if WITH_EDITOR
	_PIECleanup = FEditorDelegates::EndPIE.AddLambda([this](bool /* bIsSimulating */) { DestroyClient(); });
#endif
}


void FStormancerPlugin::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
#if WITH_EDITOR
	FEditorDelegates::EndPIE.Remove(_PIECleanup);
#endif
	DestroyClient();

	UE_LOG(StormancerLog, Display, TEXT("Unloaded Stormancer"));
}

void FStormancerPlugin::DestroyClient()
{
	if (_Client)
	{
		// Keep the client alive until it's been properly disconnected, but remove it from the plugin.
		std::shared_ptr<Stormancer::Client> client = std::move(_Client);
		try
		{
			client->disconnect().then([]()
			{
				return;
			});
		}
		catch (std::exception& e)
		{
			UE_LOG(StormancerLog, Error, TEXT("Error disconnecting the client: %s"), UTF8_TO_TCHAR(e.what()));
		}
		// The client is going to be released here
		
	}
}

std::shared_ptr<Stormancer::Client> FStormancerPlugin::CreateClient(std::string endpoint, std::string account, std::string application)
{
	if (_Client)
	{
		UE_LOG(StormancerLog, Warning, TEXT("Client already created"));
	}
	else
	{
		auto config = Stormancer::Configuration::create(endpoint, account, application);
		config->actionDispatcher = std::make_shared<FStormancerTickDispatcher>();

		_Client = Stormancer::Client::create(config);
	}

	return _Client;
}

std::shared_ptr<Stormancer::Client> FStormancerPlugin::GetClient()
{
	return _Client;
}

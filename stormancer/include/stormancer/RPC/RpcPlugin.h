#pragma once

#include "stormancer/headers.h"
#include "stormancer/IPlugin.h"
#include "stormancer/Scene.h"

namespace Stormancer
{
	class RpcPlugin : public IPlugin
	{
	public:
		void registerSceneDependencies(Scene* scene) override;

		void sceneCreated(Scene* scene) override;
		
		void sceneDisconnected(Scene* scene) override;

	public:
		STORMANCER_DLL_API static const std::string pluginName;
		STORMANCER_DLL_API static const std::string serviceName;
		STORMANCER_DLL_API static const std::string version;
		STORMANCER_DLL_API static const std::string nextRouteName;
		STORMANCER_DLL_API static const std::string errorRouteName;
		STORMANCER_DLL_API static const std::string completeRouteName;
		STORMANCER_DLL_API static const std::string cancellationRouteName;
	};
};

#pragma once

#include "stormancer/BuildConfig.h"

#include "stormancer/IPlugin.h"
#include <string>

namespace Stormancer
{
	class Scene;
	class RpcPlugin : public IPlugin
	{
	public:
		void registerSceneDependencies(ContainerBuilder& sceneBuilder, std::shared_ptr<Scene> scene) override;

		void sceneCreated(std::shared_ptr<Scene> scene) override;
		
		void sceneDisconnected(std::shared_ptr<Scene> scene) override;

	};
}

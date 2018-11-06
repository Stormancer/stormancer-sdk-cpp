#pragma once

#include "stormancer/IPlugin.h"
#include <string>

namespace Stormancer
{
	class Scene;
	class RpcPlugin : public IPlugin
	{
	public:
		void registerSceneDependencies(std::shared_ptr<Scene> scene) override;

		void sceneCreated(std::shared_ptr<Scene> scene) override;
		
		void sceneDisconnected(std::shared_ptr<Scene> scene) override;

	};
};

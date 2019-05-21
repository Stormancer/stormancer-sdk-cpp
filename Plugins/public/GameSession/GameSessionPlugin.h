#pragma once


#include "stormancer/IPlugin.h"

namespace Stormancer
{
	class GameSessionPlugin : public IPlugin
	{
	public:
		void registerClientDependencies(ContainerBuilder& builder) override;
		void registerSceneDependencies(ContainerBuilder& builder, std::shared_ptr<Scene> scene) override;
		void sceneCreated(std::shared_ptr<Scene> scene) override;
		void sceneDisconnecting(std::shared_ptr<Scene> scene) override;
	};
}

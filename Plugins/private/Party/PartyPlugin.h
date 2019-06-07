#pragma once

#include "stormancer/IPlugin.h"

namespace Stormancer
{
	class Scene;

	class PartyPlugin : public IPlugin
	{	
	public:
		void registerSceneDependencies(ContainerBuilder& builder, std::shared_ptr<Scene> scene) override;
		void sceneCreated(std::shared_ptr<Scene> scene) override;
		void registerClientDependencies(ContainerBuilder& builder) override;
	};
}

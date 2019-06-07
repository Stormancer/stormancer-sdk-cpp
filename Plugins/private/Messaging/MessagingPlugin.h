#pragma once

#include "stormancer/IPlugin.h"
#include "stormancer/Scene.h"

class IClient;
namespace Stormancer
{
	class MessagingPlugin : public IPlugin
	{
	public:

		void registerSceneDependencies(ContainerBuilder& sceneBuilder, std::shared_ptr<Scene> scene) override;
		void registerClientDependencies(ContainerBuilder& clientBuilder) override;
	};
}

#pragma once

#include "stormancer/IPlugin.h"
namespace Stormancer
{
	class Scene;
	class IClient;
}
namespace Helloworld
{
	class HelloworldPlugin : public Stormancer::IPlugin
	{
	public:

		void registerSceneDependencies(Stormancer::ContainerBuilder& builder, std::shared_ptr<Stormancer::Scene> scene) override;
		void registerClientDependencies(Stormancer::ContainerBuilder& builder) override;
	};
}

#pragma once
#include "stormancer/IPlugin.h"
#include "stormancer/Scene.h"

class IClient;
namespace Stormancer
{
	class LeaderboardPlugin : public IPlugin
	{
	public:

		void registerSceneDependencies(ContainerBuilder& builder, std::shared_ptr<Scene> scene) override;
		void registerClientDependencies(ContainerBuilder& builder) override;
	};
}

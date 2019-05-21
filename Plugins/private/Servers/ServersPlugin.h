#pragma once
#include "stormancer/IPlugin.h"
#include "stormancer/Scene.h"


class IClient;
class ContainerBuilder;

namespace Stormancer
{
	class ServerPlugin : public IPlugin
	{
	public:
		void registerClientDependencies(ContainerBuilder& builder) override;
		void registerSceneDependencies(ContainerBuilder& builder, std::shared_ptr<Scene> scene) override;
	};
}

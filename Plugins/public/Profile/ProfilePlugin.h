#pragma once
#include <memory>
#include "stormancer/IPlugin.h"

namespace Stormancer
{
	class ProfilePlugin : public IPlugin
	{
	public:
		void registerSceneDependencies(ContainerBuilder& builder, std::shared_ptr<Scene> scene) override;
		void registerClientDependencies(ContainerBuilder& builder) override;
	};
};

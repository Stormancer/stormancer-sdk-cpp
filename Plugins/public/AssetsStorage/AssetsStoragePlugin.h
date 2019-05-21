#pragma once

#include "stormancer/IPlugin.h"
#include <memory>

namespace Stormancer
{
	class AssetsStoragePlugin : public IPlugin
	{
	public:
		void registerSceneDependencies(ContainerBuilder& builder, std::shared_ptr<Scene> scene) override;
	};
}

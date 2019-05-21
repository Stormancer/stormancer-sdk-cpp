#pragma once


#include "stormancer/IPlugin.h"
#include "stormancer/Scene.h"

namespace Stormancer
{
	class GameVersionPlugin : public IPlugin
	{
	public:
		void registerSceneDependencies(ContainerBuilder& builder, std::shared_ptr<Scene> scene) override;
	};
}

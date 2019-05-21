#pragma once

#include "stormancer/IPlugin.h"

namespace Stormancer
{
	class PlayerReportPlugin : public IPlugin
	{
	public:
		void registerSceneDependencies(ContainerBuilder& builder, std::shared_ptr<Scene> scene) override;
	};
}

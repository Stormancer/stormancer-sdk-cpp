#pragma once


#include "stormancer/IPlugin.h"

namespace Stormancer
{
	class InvitationsPlugin : public IPlugin
	{
	public:
		void registerSceneDependencies(ContainerBuilder& builder, std::shared_ptr<Scene> scene) override;
	};
}
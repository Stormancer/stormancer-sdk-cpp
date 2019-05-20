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

		void sceneCreated(std::shared_ptr<Stormancer::Scene> scene) override;
		void clientCreated(std::shared_ptr<Stormancer::IClient> client) override;
	};
}

#pragma once
#include "stormancer/IPlugin.h"
#include "stormancer/Scene.h"

class IClient;
namespace Stormancer
{
	class LeaderboardPlugin : public IPlugin
	{
	public:

		void sceneCreated(std::shared_ptr<Scene> scene) override;
		void clientCreated(std::shared_ptr<IClient> client) override;
	};
}

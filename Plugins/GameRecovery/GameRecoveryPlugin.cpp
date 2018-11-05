#include "GameRecovery/GameRecoveryPlugin.h"
#include "GameRecovery/GameRecoveryService.h"
#include "GameRecovery/GameRecovery.h"

namespace Stormancer
{
	void GameRecoveryPlugin::sceneCreated(Scene* scene)
	{
		if (scene)
		{
			auto name = scene->getHostMetadata("stormancer.gameRecovery");

			if (!name.empty())
			{
				auto service = std::make_shared<GameRecoveryService>(scene);
				scene->dependencyResolver().lock()->registerDependency<GameRecoveryService>(service);
			}
		}
	}

	void GameRecoveryPlugin::clientCreated(Client* client)
	{
		if (client)
		{
			client->dependencyResolver().lock()->registerDependency<GameRecovery>([](std::weak_ptr<DependencyResolver> dr) {
				return std::make_shared<GameRecovery>(dr.lock()->resolve<AuthenticationService>()); },true);
		}
	}
}

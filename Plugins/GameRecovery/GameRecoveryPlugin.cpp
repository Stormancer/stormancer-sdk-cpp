#include "GameRecovery/GameRecoveryPlugin.h"
#include "GameRecovery/GameRecoveryService.h"
#include "GameRecovery/GameRecovery.h"

namespace Stormancer
{
	void GameRecoveryPlugin::sceneCreated(std::shared_ptr<Scene> scene)
	{
		if (scene)
		{
			auto name = scene->getHostMetadata("stormancer.gameRecovery");

			if (!name.empty())
			{
				auto service = std::make_shared<GameRecoveryService>(scene);
				scene->dependencyResolver()->registerDependency<GameRecoveryService>(service);
			}
		}
	}

	void GameRecoveryPlugin::clientCreated(std::shared_ptr<IClient> client)
	{
		if (client)
		{
			client->dependencyResolver()->registerDependency<GameRecovery>([](std::weak_ptr<DependencyResolver> dr) {
				return std::make_shared<GameRecovery>(dr.lock()->resolve<AuthenticationService>()); },true);
		}
	}
}

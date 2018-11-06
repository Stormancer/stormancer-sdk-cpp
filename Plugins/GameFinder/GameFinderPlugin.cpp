#include "GameFinder/GameFinderPlugin.h"
#include "GameFinder/GameFinderService.h"
#include "GameFinder/GameFinderManager.h"
#include "Authentication/AuthenticationService.h"

namespace Stormancer
{
	void GameFinderPlugin::sceneCreated(std::shared_ptr<Scene> scene)
	{
		if (scene)
		{
			auto name = scene->getHostMetadata("stormancer.plugins.matchmaking");
			if (!name.empty())
			{
				auto service = std::make_shared<GameFinderService>(scene);
				service->initialize();
				scene->dependencyResolver()->registerDependency<GameFinderService>(service);
			}
		}
	}
	void GameFinderPlugin::clientCreated(std::shared_ptr<IClient> client)
	{
		if (client)
		{
			client->dependencyResolver()->registerDependency<GameFinder>([](std::weak_ptr<DependencyResolver> dr) {
				return std::make_shared<GameFinder>(dr.lock()->resolve<AuthenticationService>()); });
		}
	}


};

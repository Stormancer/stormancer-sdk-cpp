#include "GameFinder/GameFinderPlugin.h"
#include "GameFinder/GameFinderService.h"
#include "GameFinder/GameFinderManager.h"
#include "Authentication/AuthenticationService.h"

namespace Stormancer
{
	void GameFinderPlugin::sceneCreated(Scene* scene)
	{
		if (scene)
		{
			auto name = scene->getHostMetadata("stormancer.plugins.matchmaking");
			if (!name.empty())
			{
				auto service = std::make_shared<GameFinderService>(scene->shared_from_this());
				service->initialize();
				scene->dependencyResolver().lock()->registerDependency<GameFinderService>(service);
			}
		}
	}
	void GameFinderPlugin::clientCreated(Client* client)
	{
		if (client)
		{
			client->dependencyResolver().lock()->registerDependency<GameFinder>([](std::weak_ptr<DependencyResolver> dr) {
				return std::make_shared<GameFinder>(dr.lock()->resolve<AuthenticationService>()); });
		}
	}


};

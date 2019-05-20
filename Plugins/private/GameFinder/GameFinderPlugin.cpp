#if defined(STORMANCER_CUSTOM_PCH)
#include STORMANCER_CUSTOM_PCH
#endif
#include "GameFinder/GameFinderPlugin.h"
#include "GameFinderService.h"
#include "GameFinder/GameFinder.h"
#include "GameFinder_Impl.h"
#include "Authentication/AuthenticationService.h"

namespace Stormancer
{
	void GameFinderPlugin::sceneCreated(std::shared_ptr<Scene> scene)
	{
		if (scene)
		{
			auto name = scene->getHostMetadata("stormancer.plugins.gamefinder");
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
				return std::make_shared<GameFinder_Impl>(dr.lock()->resolve<AuthenticationService>()); },true);
		}
	}


};

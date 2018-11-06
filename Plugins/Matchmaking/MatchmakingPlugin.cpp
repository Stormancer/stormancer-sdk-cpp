#include "stormancer/headers.h"
#include "MatchmakingPlugin.h"
#include "MatchmakingService.h"

namespace Stormancer
{
	void MatchmakingPlugin::sceneCreated(std::shared_ptr<Scene> scene)
	{
		if (scene)
		{
			auto name = scene->getHostMetadata("stormancer.plugins.matchmaking");

			if (name.length() > 0 )
			{
				auto service = std::make_shared<MatchmakingService>(scene);
				scene->dependencyResolver()->registerDependency<MatchmakingService>(service);
			}
		}
	}
};

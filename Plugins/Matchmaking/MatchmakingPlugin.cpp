#include <stdafx.h>
#include "MatchmakingPlugin.h"
#include "MatchmakingService.h"

namespace Stormancer
{
	void MatchmakingPlugin::sceneCreated(Scene* scene)
	{
		if (scene)
		{
			auto name = scene->getHostMetadata("stormancer.plugins.matchmaking");

			if (name.length() > 0 )
			{
				auto service = std::make_shared<MatchmakingService>(scene->shared_from_this());
				scene->dependencyResolver()->registerDependency<MatchmakingService>(service);
			}
		}
	}
};

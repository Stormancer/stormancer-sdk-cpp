#include "stormancer/headers.h"
#include "LeaderboardPlugin.h"
#include "LeaderboardService.h"

namespace Stormancer
{
	void LeaderboardPlugin::sceneCreated(Scene* scene)
	{
		if (scene)
		{
			auto name = scene->getHostMetadata("stormancer.leaderboard");

			if (!name.empty())
			{
				auto service = std::make_shared<LeaderboardService>(scene);
				scene->dependencyResolver()->registerDependency<LeaderboardService>(service);
			}
		}
	}
}

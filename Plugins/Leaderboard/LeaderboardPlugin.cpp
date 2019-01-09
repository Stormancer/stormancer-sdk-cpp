#include "stormancer/headers.h"
#include "Leaderboard/LeaderboardPlugin.h"
#include "Leaderboard/LeaderboardService.h"
#include "Leaderboard/Leaderboard.h"
namespace Stormancer
{
	void LeaderboardPlugin::sceneCreated(std::shared_ptr<Scene> scene)
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
	void LeaderboardPlugin::clientCreated(std::shared_ptr<IClient> client)
	{
		if (client)
		{
			client->dependencyResolver()->registerDependency<Leaderboard>([](std::weak_ptr<DependencyResolver> dr) {
				return std::make_shared<Leaderboard>(dr.lock()->resolve<AuthenticationService>()); });
		}
	}
}

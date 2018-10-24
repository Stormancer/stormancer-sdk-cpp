#include "stormancer/headers.h"
#include "Leaderboard/LeaderboardPlugin.h"
#include "Leaderboard/LeaderboardService.h"
#include "Leaderboard/Leaderboard.h"
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
				scene->dependencyResolver().lock()->registerDependency<LeaderboardService>(service);
			}
		}
	}
	void LeaderboardPlugin::clientCreated(Client* client)
	{
		if (client)
		{
			client->dependencyResolver().lock()->registerDependency<Leaderboard>([](std::weak_ptr<DependencyResolver> dr) {
				return std::make_shared<Leaderboard>(dr.lock()->resolve<AuthenticationService>()); });
		}
	}
}

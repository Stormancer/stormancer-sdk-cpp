#if defined(STORMANCER_CUSTOM_PCH)
#include STORMANCER_CUSTOM_PCH
#endif
#include "Leaderboard/LeaderboardPlugin.h"

#include "LeaderboardService.h"
#include "Leaderboard_Impl.h"
#include "Leaderboard/Leaderboard.h"
#include "stormancer/IClient.h"

namespace Stormancer
{
	void LeaderboardPlugin::registerSceneDependencies(ContainerBuilder& builder, std::shared_ptr<Scene> scene)
	{
		if (scene)
		{
			auto name = scene->getHostMetadata("stormancer.leaderboard");

			if (!name.empty())
			{
				builder.registerDependency<LeaderboardService, Scene>().singleInstance();
			}
		}
	}

	void LeaderboardPlugin::registerClientDependencies(ContainerBuilder& builder)
	{
		builder.registerDependency<Leaderboard_Impl, Users::UsersApi>().as<Leaderboard>().singleInstance();
	}
}

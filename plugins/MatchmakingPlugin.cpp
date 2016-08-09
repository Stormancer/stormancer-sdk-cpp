#include "MatchmakingPlugin.h"
#include "MatchmakingService.h"
#include "GameSession.h"
namespace Stormancer
{
	void MatchmakingPlugin::sceneCreated(Stormancer::Scene* scene)
	{
		if (scene)
		{
			auto name = scene->getHostMetadata("stormancer.plugins.matchmaking");

			if (name && std::strlen(name) > 0 )
			{
				auto service = new MatchmakingService(scene);
				scene->dependencyResolver()->registerDependency<MatchmakingService>(service);
			}

			name = scene->getHostMetadata("stormancer.gamesession");
			if (name && std::strlen(name) > 0)
			{
				auto service = new GameSessionService(scene);
				scene->dependencyResolver()->registerDependency<GameSessionService>(service);
			}
		}
	}

	void MatchmakingPlugin::destroy()
	{
		delete this;
	}
};

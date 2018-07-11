#include "stormancer/headers.h"
#include "GameSessionPlugin.h"
#include "GameSessionService.h"

namespace Stormancer
{
	void GameSessionPlugin::sceneCreated(Scene* scene)
	{
		if (scene)
		{
			auto name = scene->getHostMetadata("stormancer.gamesession");
			if (name.length() > 0)
			{
				auto service = std::make_shared<GameSessionService>(scene->shared_from_this());
				scene->dependencyResolver().lock()->registerDependency<GameSessionService>(service);
			}
		}
	}
	void GameSessionPlugin::sceneDisconnecting(Scene * scene)
	{
		if (scene)
		{
			auto name = scene->getHostMetadata("stormancer.gamesession");
			if (name.length() > 0)
			{
				auto gameSession = scene->dependencyResolver().lock()->resolve<GameSessionService>();
				if (gameSession)
				{
					gameSession->__disconnecting();
				}
			}
		}
	}
};

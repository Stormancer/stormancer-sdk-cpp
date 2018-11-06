#include "stormancer/headers.h"
#include "GameSessionPlugin.h"
#include "GameSessionService.h"

namespace Stormancer
{
	void GameSessionPlugin::sceneCreated(std::shared_ptr<Scene> scene)
	{
		if (scene)
		{
			auto name = scene->getHostMetadata("stormancer.gamesession");
			if (name.length() > 0)
			{
				auto service = std::make_shared<GameSessionService>(scene);
				scene->dependencyResolver()->registerDependency<GameSessionService>(service);
			}
		}
	}
	void GameSessionPlugin::sceneDisconnecting(std::shared_ptr<Scene> scene)
	{
		if (scene)
		{
			auto name = scene->getHostMetadata("stormancer.gamesession");
			if (name.length() > 0)
			{
				auto gameSession = scene->dependencyResolver()->resolve<GameSessionService>();
				if (gameSession)
				{
					gameSession->__disconnecting();
				}
			}
		}
	}
};

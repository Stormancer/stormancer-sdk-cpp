#if defined(STORMANCER_CUSTOM_PCH)
#include STORMANCER_CUSTOM_PCH
#endif
#include "GameSession/GameSessionPlugin.h"

#include "GameSession/GameSessionService.h"
#include "GameSession_Impl.h"
#include "stormancer/IClient.h"

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
				service->Initialize();
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
					gameSession->onDisconnecting();
				}
			}
		}
	}

	void GameSessionPlugin::clientCreated(std::shared_ptr<IClient> client)
	{
		if (client)
		{
			std::weak_ptr<IClient> wClient = client;
			client->dependencyResolver()->registerDependency<GameSession>([wClient](std::weak_ptr<DependencyResolver> dr) {
				return std::make_shared<GameSession_Impl>(wClient);
			}, true);

		}
	}
};

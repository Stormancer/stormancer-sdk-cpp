#include "GameSession/GameSessionPlugin.h"
#include "GameSessionService.h"
#include "GameSession_Impl.h"
#include "stormancer/IClient.h"

namespace Stormancer
{
	void GameSessionPlugin::registerSceneDependencies(ContainerBuilder& builder, std::shared_ptr<Scene> scene)
	{
		if (scene)
		{
			auto name = scene->getHostMetadata("stormancer.gamesession");
			if (name.length() > 0)
			{
				builder.registerDependency<GameSessionService, Scene>().singleInstance();
			}
		}
	}

	void GameSessionPlugin::sceneCreated(std::shared_ptr<Scene> scene)
	{
		if (scene)
		{
			auto name = scene->getHostMetadata("stormancer.gamesession");
			if (name.length() > 0)
			{
				scene->dependencyResolver().resolve<GameSessionService>()->initialize();
				scene->dependencyResolver().resolve<GameSession>()->onConnectingToScene(scene);
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
				auto gameSession = scene->dependencyResolver().resolve<GameSessionService>();
				if (gameSession)
				{
					scene->dependencyResolver().resolve<GameSession>()->onDisconnectingFromScene(scene);
					gameSession->onDisconnecting();
				}
			}
		}
	}

	void GameSessionPlugin::registerClientDependencies(ContainerBuilder& builder)
	{
		builder.registerDependency<GameSession_Impl, IClient, ILogger>().as<GameSession>().singleInstance();
	}
};

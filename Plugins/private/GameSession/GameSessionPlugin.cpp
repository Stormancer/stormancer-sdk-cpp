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
		scene->dependencyResolver().resolve<GameSessionService>()->initialize();
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
					gameSession->onDisconnecting();
				}
			}
		}
	}

	void GameSessionPlugin::registerClientDependencies(ContainerBuilder& builder)
	{
		builder.registerDependency<GameSession_Impl, IClient>().as<GameSession>().singleInstance();
	}
};

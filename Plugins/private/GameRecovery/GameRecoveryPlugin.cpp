#if defined(STORMANCER_CUSTOM_PCH)
#include STORMANCER_CUSTOM_PCH
#endif
#include "GameRecoveryPlugin.h"
#include "GameRecovery/GameRecovery.h"
#include "Authentication/AuthenticationService.h"
#include "GameRecoveryService.h"
#include "GameRecovery_Impl.h"

namespace Stormancer
{
	void GameRecoveryPlugin::registerSceneDependencies(ContainerBuilder& builder, std::shared_ptr<Scene> scene)
	{
		if (scene)
		{
			auto name = scene->getHostMetadata("stormancer.gameRecovery");

			if (!name.empty())
			{
				builder.registerDependency<GameRecoveryService, Scene>().singleInstance();
			}
		}
	}

	void GameRecoveryPlugin::registerClientDependencies(ContainerBuilder& builder)
	{
		builder.registerDependency<GameRecovery_Impl, AuthenticationService>().as<GameRecovery>().singleInstance();
	}
}

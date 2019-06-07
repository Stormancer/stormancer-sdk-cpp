#if defined(STORMANCER_CUSTOM_PCH)
#include STORMANCER_CUSTOM_PCH
#endif
#include "GameVersion/GameVersionPlugin.h"
#include "GameVersion/GameVersionService.h"

namespace Stormancer
{
	void GameVersionPlugin::registerSceneDependencies(ContainerBuilder& builder, std::shared_ptr<Scene> scene)
	{
		if (scene)
		{
			auto name = scene->getHostMetadata("stormancer.gameVersion");

			if (!name.empty())
			{
				builder.registerDependency<GameVersionService, Scene>().singleInstance();
			}
		}
	}
}

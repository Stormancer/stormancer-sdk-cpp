#if defined(STORMANCER_CUSTOM_PCH)
#include STORMANCER_CUSTOM_PCH
#endif
#include "GameVersion/GameVersionPlugin.h"
#include "GameVersion/GameVersionService.h"

namespace Stormancer
{
	void GameVersionPlugin::sceneCreated(std::shared_ptr<Scene> scene)
	{
		if (scene)
		{
			auto name = scene->getHostMetadata("stormancer.gameVersion");

			if (!name.empty())
			{
				auto service = std::make_shared<GameVersionService>(scene);
				scene->dependencyResolver()->registerDependency<GameVersionService>(service);
			}
		}
	}
}

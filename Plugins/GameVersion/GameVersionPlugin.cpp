#include "stdafx.h"
#include "GameVersionPlugin.h"
#include "GameVersionService.h"

namespace Stormancer
{
	void GameVersionPlugin::sceneCreated(Scene* scene)
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

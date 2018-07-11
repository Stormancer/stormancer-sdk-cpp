#include "Stormancer/stdafx.h"
#include "stormancer/headers.h"
#include "AssetsStoragePlugin.h"
#include "AssetsStorageService.h"

namespace Stormancer
{
	void AssetsStoragePlugin::sceneCreated(Scene* scene)
	{
		if (scene)
		{	
			auto name = scene->getHostMetadata("stormancer.assetsstorage");
			if (!name.empty())
			{
				auto logger = scene->dependencyResolver().lock()->resolve<ILogger>();
				auto service = std::make_shared<AssetsStorageService>(scene->shared_from_this(), logger);
				scene->dependencyResolver().lock()->registerDependency<AssetsStorageService>(service);
			}
		}
	}
};

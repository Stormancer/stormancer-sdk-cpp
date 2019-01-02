#if defined(STORMANCER_CUSTOM_PCH)
#include STORMANCER_CUSTOM_PCH
#endif
#include "AssetsStorage/AssetsStoragePlugin.h"
#include "stormancer/headers.h"
#include "stormancer/Logger/ILogger.h"
#include "AssetsStorage/AssetsStorageService.h"

namespace Stormancer
{
	void AssetsStoragePlugin::sceneCreated(std::shared_ptr<Scene> scene)
	{
		if (scene)
		{	
			auto name = scene->getHostMetadata("stormancer.assetsstorage");
			if (!name.empty())
			{
				auto logger = scene->dependencyResolver()->resolve<ILogger>();
				auto service = std::make_shared<AssetsStorageService>(scene, logger);
				scene->dependencyResolver()->registerDependency<AssetsStorageService>(service);
			}
		}
	}
};

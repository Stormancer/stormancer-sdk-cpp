#include "AssetsStorage/AssetsStoragePlugin.h"
#include "stormancer/Logger/ILogger.h"
#include "AssetsStorage/AssetsStorageService.h"
#include "stormancer/Scene.h"

namespace Stormancer
{
	void AssetsStoragePlugin::registerSceneDependencies(ContainerBuilder& builder, std::shared_ptr<Scene> scene)
	{
		if (scene)
		{	
			auto name = scene->getHostMetadata("stormancer.assetsstorage");
			if (!name.empty())
			{
				builder.registerDependency<AssetsStorageService, Scene, ILogger>();
			}
		}
	}
};

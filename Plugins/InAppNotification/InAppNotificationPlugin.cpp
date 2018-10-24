#include "Stormancer/stdafx.h"
#include "InAppNotificationPlugin.h"
#include "InAppNotificationService.h"

namespace Stormancer
{
	void InAppNotificationPlugin::sceneCreated(Scene* scene)
	{
		if (scene)
		{
			auto name = scene->getHostMetadata("stormancer.inappnotification");

			if (!name.empty())
			{
				auto service = std::make_shared<InAppNotificationService>(scene);
				scene->dependencyResolver().lock()->registerDependency<InAppNotificationService>(service);
			}
		}
	}
}

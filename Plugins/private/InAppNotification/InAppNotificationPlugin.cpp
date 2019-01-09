#if defined(STORMANCER_CUSTOM_PCH)
#include STORMANCER_CUSTOM_PCH
#endif
#include "InAppNotification/InAppNotificationPlugin.h"
#include "InAppNotification/InAppNotificationService.h"

namespace Stormancer
{
	void InAppNotificationPlugin::sceneCreated(std::shared_ptr<Scene> scene)
	{
		if (scene)
		{
			auto name = scene->getHostMetadata("stormancer.inappnotification");

			if (!name.empty())
			{
				scene->dependencyResolver()->registerDependency<InAppNotificationService>([](auto dr) {
					auto service = std::make_shared<InAppNotificationService>(dr.lock()->template resolve<Scene>());
					service->initialize();
					return service;
				}, true);
			}
		}
	}
}

#if defined(STORMANCER_CUSTOM_PCH)
#include STORMANCER_CUSTOM_PCH
#endif
#include "InAppNotification/InAppNotificationPlugin.h"
#include "InAppNotification/InAppNotificationService.h"

namespace Stormancer
{
	void InAppNotificationPlugin::registerSceneDependencies(ContainerBuilder& builder, std::shared_ptr<Scene> scene)
	{
		if (scene)
		{
			auto name = scene->getHostMetadata("stormancer.inappnotification");

			if (!name.empty())
			{
				builder.registerDependency<InAppNotificationService>([](const DependencyScope& dr) {
					auto service = std::make_shared<InAppNotificationService>(dr.resolve<Scene>());
					service->initialize();
					return service;
				}).singleInstance();
			}
		}
	}
}

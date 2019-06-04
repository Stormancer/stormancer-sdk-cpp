#if defined(STORMANCER_CUSTOM_PCH)
#include STORMANCER_CUSTOM_PCH
#endif
#include "MessagingPlugin.h"
#include "stormancer/Logger/ILogger.h"
#include "stormancer/IClient.h"
#include "MessagingService.h"
#include "Messaging_Impl.h"

namespace Stormancer
{
	void Stormancer::MessagingPlugin::registerClientDependencies(ContainerBuilder & clientBuilder)
	{
		clientBuilder.registerDependency<Messaging, ILogger>();
	}

	void Stormancer::MessagingPlugin::registerSceneDependencies(ContainerBuilder & sceneBuilder, std::shared_ptr<Scene> scene)
	{
		if (scene)
		{
			auto name = scene->getHostMetadata("stormancer.messaging");

			if (!name.empty())
			{
				sceneBuilder.registerDependency<MessagingService, Scene>();
			}
		}
	}
}

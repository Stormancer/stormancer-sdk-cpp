#if defined(STORMANCER_CUSTOM_PCH)
#include STORMANCER_CUSTOM_PCH
#endif
#include "Members/FriendsPlugin.h"
#include "Members/Friends.h"
#include "Friends_Impl.h"
#include "FriendsService.h"

namespace Stormancer
{
	void FriendsPlugin::registerSceneDependencies(ContainerBuilder& builder, std::shared_ptr<Scene> scene)
	{
		if (scene)
		{
			auto name = scene->getHostMetadata("stormancer.friends");
			if (name.length() > 0)
			{
				builder.registerDependency<FriendsService, Scene, ILogger>().singleInstance();
			}
		}
	}

	void FriendsPlugin::registerClientDependencies(ContainerBuilder& builder)
	{
		builder.registerDependency<Friends_Impl, AuthenticationService>().as<Friends>().singleInstance();
	}
}

#include "stormancer/headers.h"
#include "FriendsPlugin.h"
#include "Friends.h"

namespace Stormancer
{
	void FriendsPlugin::sceneCreated(Scene* scene)
	{
		if (scene)
		{
			auto name = scene->getHostMetadata("stormancer.friends");
			if (name.length() > 0)
			{
				
				scene->dependencyResolver()->registerDependency<FriendsService>([scene](DependencyResolver* dr) {
					return std::make_shared<FriendsService>(scene->shared_from_this(), dr->resolve<ILogger>());
				},true);
			}
		}
	}
};

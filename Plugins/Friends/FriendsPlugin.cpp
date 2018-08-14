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
				
				scene->dependencyResolver().lock()->registerDependency<FriendsService>([scene](std::weak_ptr<DependencyResolver> dr) {
					return std::make_shared<FriendsService>(scene->shared_from_this(), dr.lock()->resolve<ILogger>());
				},true);
			}
		}
	}
};

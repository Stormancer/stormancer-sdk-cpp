#include "stormancer/headers.h"
#include "FriendsPlugin.h"
#include "Friends.h"

namespace Stormancer
{
	void FriendsPlugin::sceneCreated(std::shared_ptr<Scene> scene)
	{
		if (scene)
		{
			auto name = scene->getHostMetadata("stormancer.friends");
			if (name.length() > 0)
			{
				
				scene->dependencyResolver()->registerDependency<FriendsService>([scene](std::weak_ptr<DependencyResolver> dr) {
					return std::make_shared<FriendsService>(scene, dr.lock()->resolve<ILogger>());
				},true);
			}
		}
	}

	void FriendsPlugin::clientCreated(std::shared_ptr<IClient> client)
	{
		if (client)
		{
			client->dependencyResolver()->registerDependency<Friends>([](std::weak_ptr<DependencyResolver> dr) {
				return std::make_shared<Friends>(dr.lock()->resolve<AuthenticationService>()); 
			}, true);

		}
	}
};

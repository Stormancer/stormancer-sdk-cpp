#include "Profile/ProfilePlugin.h"
#include "Profile/ProfileService.h"
#include "Profile/Profiles.h"

namespace Stormancer
{
	ProfilePlugin::ProfilePlugin()
	{
	}

	ProfilePlugin::~ProfilePlugin()
	{
	}

	void ProfilePlugin::sceneCreated(Scene* scene)
	{
		if (scene)
		{
			auto name = scene->getHostMetadata("stormancer.profiles");
			if (!name.empty())
			{
				std::shared_ptr<ProfileService> service = std::make_shared<ProfileService>(scene->shared_from_this());
				scene->dependencyResolver().lock()->registerDependency<ProfileService>(service);
			}
		}
	}
	
	void ProfilePlugin::clientCreated(Client* client)
	{
		if (client)
		{
			client->dependencyResolver().lock()->registerDependency<Profiles>([](std::weak_ptr<DependencyResolver> dr) {
				return std::make_shared<Profiles>(dr.lock()->resolve<AuthenticationService>()); },true);
		}
	}
};

#include "Profile/ProfilePlugin.h"
#include "Profile/ProfileService.h"

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
};

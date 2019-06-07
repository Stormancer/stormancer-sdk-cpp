#if defined(STORMANCER_CUSTOM_PCH)
#include STORMANCER_CUSTOM_PCH
#endif
#include "Profile/ProfilePlugin.h"
#include "ProfileService.h"
#include "Profiles_Impl.h"
#include "Authentication/AuthenticationService.h"
#include "stormancer/Scene.h"

namespace Stormancer
{
	void ProfilePlugin::registerSceneDependencies(ContainerBuilder& builder, std::shared_ptr<Scene> scene)
	{
		if (scene)
		{
			auto name = scene->getHostMetadata("stormancer.profiles");
			if (!name.empty())
			{
				builder.registerDependency<ProfileService, Scene>().singleInstance();
			}
		}
	}
	
	void ProfilePlugin::registerClientDependencies(ContainerBuilder& builder)
	{
		builder.registerDependency<Profiles_Impl, AuthenticationService>().as<Profiles>().singleInstance();
	}
}

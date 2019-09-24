#include "OrganizationsPlugin.h"

#include "OrganizationsPlugin.h"
#include "OrganizationsService.h"
#include "Organizations_Impl.h"
#include "stormancer/Scene.h"
#include "Users/Users.hpp"

namespace Stormancer
{
	void OrganizationsPlugin::registerSceneDependencies(ContainerBuilder& builder, std::shared_ptr<Scene> scene)
	{
		if (scene)
		{
			auto name = scene->getHostMetadata("stormancer.organizations");

			if (!name.empty())
			{
				builder.registerDependency<OrganizationsService, Scene>().singleInstance();
			}
		}
	}

	void OrganizationsPlugin::registerClientDependencies(ContainerBuilder& builder)
	{
		builder.registerDependency<Organizations>([](const DependencyScope& scope)
		{
			auto usersService = scope.resolve<Users::UsersApi>();
			auto organizations = std::make_shared<Organizations_Impl>(usersService);
			organizations->initialize();
			return organizations;
		}).singleInstance();
	}
}

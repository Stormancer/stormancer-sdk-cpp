#include "OrganizationsPlugin.h"

#include "OrganizationsPlugin.h"
#include "OrganizationsService.h"
#include "Organizations_Impl.h"
#include "stormancer/Scene.h"
#include "Authentication/AuthenticationService.h"

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
			auto authService = scope.resolve<AuthenticationService>();
			auto organizations = std::make_shared<Organizations_Impl>(authService);
			organizations->initialize();
			return organizations;
		}).singleInstance();
	}
}

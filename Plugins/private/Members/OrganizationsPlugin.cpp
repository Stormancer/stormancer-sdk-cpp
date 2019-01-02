#include "OrganizationsPlugin.h"

#include "OrganizationsPlugin.h"
#include "OrganizationsService.h"
#include "Organizations_Impl.h"
#include "stormancer/Scene.h"
#include "Authentication/AuthenticationService.h"

namespace Stormancer
{
	void OrganizationsPlugin::sceneCreated(std::shared_ptr<Scene> scene)
	{
		if (scene)
		{
			auto name = scene->getHostMetadata("stormancer.organizations");

			if (!name.empty())
			{
				auto service = std::make_shared<OrganizationsService>(scene);
				scene->dependencyResolver()->registerDependency<OrganizationsService>(service);
			}
		}
	}

	void OrganizationsPlugin::clientCreated(std::shared_ptr<IClient> client)
	{
		client->dependencyResolver()->registerDependency<Organizations>([](std::weak_ptr<DependencyResolver> dr)
		{
			auto authService = dr.lock()->resolve<AuthenticationService>();
			auto organizations = std::make_shared<Organizations_Impl>(authService);
			organizations->initialize();
			return organizations;
		}, true);
	}
}

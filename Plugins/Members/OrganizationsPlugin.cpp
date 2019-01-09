#include "stormancer/headers.h"
#include "OrganizationsPlugin.h"
#include "OrganizationsService.h"
#include "Organizations.h"
#include "stormancer/Scene.h"

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
		if (client)
		{
			client->dependencyResolver()->registerDependency<Organizations>([](std::weak_ptr<DependencyResolver> dr)
			{
				auto authService = dr.lock()->resolve<AuthenticationService>();
				auto organizations = std::make_shared<Organizations>(authService);
				organizations->initialize();
				return organizations;
			}, true);
		}
	}
}

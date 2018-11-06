#include "stormancer/headers.h"
#include "OrganizationsPlugin.h"
#include "OrganizationsService.h"
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
}

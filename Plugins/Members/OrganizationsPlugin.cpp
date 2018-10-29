#include "stormancer/headers.h"
#include "OrganizationsPlugin.h"
#include "OrganizationsService.h"
#include "stormancer/Scene.h"

namespace Stormancer
{
	void OrganizationsPlugin::sceneCreated(Scene* scene)
	{
		if (scene)
		{
			auto name = scene->getHostMetadata("stormancer.organizations");

			if (!name.empty())
			{
				auto service = std::make_shared<OrganizationsService>(scene->shared_from_this());
				scene->dependencyResolver().lock()->registerDependency<OrganizationsService>(service);
			}
		}
	}
}

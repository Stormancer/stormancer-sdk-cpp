#include "stormancer/headers.h"
#include "OrganizationsContainer.h"
#include "OrganizationsService.h"
#include "stormancer/Scene.h"

namespace Stormancer
{
	std::shared_ptr<OrganizationsService> OrganizationsContainer::service()
	{
		if (auto scene = _scene.lock())
		{
			return scene->dependencyResolver().lock()->resolve<OrganizationsService>();
		}

		return nullptr;
	}
}

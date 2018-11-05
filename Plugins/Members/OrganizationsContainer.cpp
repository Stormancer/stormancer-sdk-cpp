#include "stormancer/headers.h"
#include "OrganizationsContainer.h"
#include "OrganizationsService.h"
#include "stormancer/Scene.h"
#include "stormancer/SafeCapture.h"

namespace Stormancer
{
	std::shared_ptr<OrganizationsService> OrganizationsContainer::service()
	{
		auto scene = _scene.lock();

		if (!scene)
		{
			throw PointerDeletedException("Scene deleted");
		}

		return scene->dependencyResolver().lock()->resolve<OrganizationsService>();
	}
}

#include "stormancer/headers.h"
#include "TeamsContainer.h"
#include "TeamsService.h"
#include "stormancer/Scene.h"

namespace Stormancer
{
	std::shared_ptr<TeamsService> TeamsContainer::service()
	{
		if (auto scene = _scene.lock())
		{
			return scene->dependencyResolver().lock()->resolve<TeamsService>();
		}

		return nullptr;
	}
}

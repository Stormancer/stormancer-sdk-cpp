#if defined(STORMANCER_CUSTOM_PCH)
#include STORMANCER_CUSTOM_PCH
#endif
#include "InAppNotification/InAppNotificationContainer.h"
#include "stormancer/Scene.h"
#include "Users/Users.hpp"
#include "stormancer/Exceptions.h"

namespace Stormancer
{
	InAppNotificationContainer::InAppNotificationContainer(std::shared_ptr<Scene> scene)
		: _scene(scene)
	{
	}

	std::shared_ptr<InAppNotificationService> InAppNotificationContainer::service()
	{
		auto scene = _scene.lock();

		if (!scene)
		{
			throw PointerDeletedException("Scene deleted");
		}

		return scene->dependencyResolver().resolve<InAppNotificationService>();
	}
}
#pragma once

#include "stormancer/Event.h"
#include "InAppNotification/InAppNotificationModels.h"
#include "InAppNotification/InAppNotificationService.h"
#include "stormancer/Scene.h"

namespace Stormancer
{
	class InAppNotificationContainer
	{
		friend class InAppNotification_Impl;

		public:

			InAppNotificationContainer(std::shared_ptr<Scene> scene);

			std::shared_ptr<InAppNotificationService> service();

		private:

			std::weak_ptr<Scene> _scene;
			rxcpp::composite_subscription _connectionStateChangedSubscription;
	};
}

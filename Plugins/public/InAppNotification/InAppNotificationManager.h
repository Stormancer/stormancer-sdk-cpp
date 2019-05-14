#pragma once

#include "InAppNotification/InAppNotificationModels.h"
#include "stormancer/Event.h"

namespace Stormancer
{
	class InAppNotificationContainer;

	class InAppNotificationManager
	{
	public:

		virtual ~InAppNotificationManager() {}

		virtual void initialize() = 0;

		virtual Event<InAppNotification>::Subscription subscribeNotificationReceived(std::function<void(InAppNotification)> callback) = 0;

		virtual void notificationReceived(const InAppNotification& notification) = 0;

		virtual pplx::task<void> acknowledgeNotification(const std::string& notificationId) = 0;
	};
}

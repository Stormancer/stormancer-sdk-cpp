#pragma once

#include "core/ClientAPI.h"
#include "stormancer/Event.h"
#include "InAppNotification/InAppNotificationManager.h"
#include "InAppNotification/InAppNotificationModels.h"
#include "InAppNotification/InAppNotificationService.h"
#include "Authentication/AuthenticationService.h"

namespace Stormancer
{
	class InAppNotification_Impl : public ClientAPI<InAppNotification_Impl>, public InAppNotificationManager
	{
	public:

		InAppNotification_Impl(std::weak_ptr<AuthenticationService> auth, std::shared_ptr<ILogger> logger);		

		void initialize() override;

		Event<InAppNotification>::Subscription subscribeNotificationReceived(std::function<void(InAppNotification)> callback) override;

		void notificationReceived(const InAppNotification& notification) override;

		pplx::task<void> acknowledgeNotification(const std::string& notificationId) override;

	private:

		std::shared_ptr<ILogger> _logger;
		pplx::task<std::shared_ptr<InAppNotificationContainer>> _notificationContainerTask;
		Event<InAppNotification> _onNotificationReceived;
		std::weak_ptr<AuthenticationService> _auth;
	};
}
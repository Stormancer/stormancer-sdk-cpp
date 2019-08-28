#if defined(STORMANCER_CUSTOM_PCH)
#include STORMANCER_CUSTOM_PCH
#endif
#include "InAppNotification/InAppNotification_Impl.h"
#include "InAppNotification/InAppNotificationService.h"
#include "InAppNotification/InAppNotificationContainer.h"
#include "stormancer/Exceptions.h"

namespace Stormancer
{
	InAppNotification_Impl::InAppNotification_Impl(std::weak_ptr<Users::UsersApi> users, std::shared_ptr<ILogger> logger)
		: ClientAPI(users)
		, _users(users)
		, _logger(logger)
	{
	}

	void InAppNotification_Impl::initialize()
	{
		auto users = _users.lock();

		if (!users)
		{
			_logger->log(LogLevel::Error, "InAppNotification", "Can't initialize InAppNotification : Authentication service destroyed.");
			return;
		}

		auto logger = _logger;
		std::weak_ptr<InAppNotification_Impl> wNotifications = this->shared_from_this();
		_notificationContainerTask = users->getSceneForService("stormancer.plugins.notifications")
			.then([wNotifications](std::shared_ptr<Scene> scene)
		{
			auto notifications = wNotifications.lock();
			if (!notifications)
			{
				throw PointerDeletedException("Notifications service destroyed.");
			}

			auto container = std::make_shared<InAppNotificationContainer>(scene);
			container->_connectionStateChangedSubscription = scene->getConnectionStateChangedObservable()
				.subscribe([wNotifications](ConnectionState s)
			{
				if (s == ConnectionState::Disconnected)
				{
					if (auto notifications = wNotifications.lock())
					{
						notifications->_notificationContainerTask = pplx::task_from_result<std::shared_ptr<InAppNotificationContainer>>(nullptr);
					}
				}
			});
			return container;
		});
		_notificationContainerTask.then([logger,wNotifications](pplx::task<std::shared_ptr<InAppNotificationContainer>> t)
		{
			try
			{
				if (auto notifications = wNotifications.lock())
				{
					auto container = t.get();
					container->service()->registerNotificationsCallback([wNotifications](InAppNotification notif) {
						if (auto notifications = wNotifications.lock())
						{
							notifications->notificationReceived(notif);
						}
					});
				}
			}
			catch (const std::exception& ex)
			{
				logger->log(LogLevel::Error, "InAppNotification", "InAppNotification initialization failed", ex.what());
			}
		});
	}

	Event<InAppNotification>::Subscription InAppNotification_Impl::subscribeNotificationReceived(std::function<void(InAppNotification)> callback)
	{
		return _onNotificationReceived.subscribe(callback);
	}

	void InAppNotification_Impl::notificationReceived(const InAppNotification& notification)
	{
		_onNotificationReceived(notification);
	}

	pplx::task<void> InAppNotification_Impl::acknowledgeNotification(const std::string& notificationId)
	{
		return _notificationContainerTask
			.then([notificationId](std::shared_ptr<InAppNotificationContainer> notificationContainer)
		{
			return notificationContainer->service()->acknowledgeNotification(notificationId);
		});
	}
}

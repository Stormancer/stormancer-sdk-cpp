#pragma once

#include "InAppNotification/InAppNotificationModels.h"
#include "stormancer/RPC/Service.h"

namespace Stormancer
{
	class InAppNotificationPlugin;

	class InAppNotificationService : public std::enable_shared_from_this<InAppNotificationService>
	{
		friend InAppNotificationPlugin;
	public:

#pragma region public_methods

		InAppNotificationService(std::shared_ptr<Scene> scene);

		// Register a callback to process the incoming notifications
		void registerNotificationsCallback(const std::function<void(InAppNotification)>& callback);

		// Push a notification (this will fire the registered callback)
		void notificationReceived(const InAppNotification& notification);

		// Acknowledge a notification (this is automatically done if the acknowledgement is not 'ByUser')
		pplx::task<void> acknowledgeNotification(const std::string& notificationId);

#pragma endregion

	private:
		void initialize();
#pragma region

		std::shared_ptr<ILogger> _logger;
		std::weak_ptr<Scene> _scene;
		std::shared_ptr<RpcService> _rpcService;
		std::function<void(const InAppNotification&)> _callback;
		std::queue<InAppNotification> _pendingNotifications;
		std::weak_ptr<IActionDispatcher> _dispatcher;
		std::mutex _lock;

#pragma endregion
	};
}

#pragma once
#include "Stormancer/headers.h"
#include "Stormancer/RPC/Service.h"
#include "InAppNotification.h"

namespace Stormancer
{
	class InAppNotificationPlugin;

	class InAppNotificationService : public std::enable_shared_from_this<InAppNotificationService>
	{
		friend InAppNotificationPlugin;
	public:

#pragma region public_methods

		InAppNotificationService(std::shared_ptr<Scene> scene);

		void registerNotificationsCallback(const std::function<void(InAppNotification)>& callback);

		void notificationReceived(const InAppNotification& notification);

		pplx::task<void> acknowledgeNotification(const std::string& notificationId);

#pragma endregion

	private:
		void initialize();
#pragma region

		std::weak_ptr<Scene> _scene;
		std::shared_ptr<RpcService> _rpcService;
		std::function<void(const InAppNotification&)> _callback;
		std::queue<InAppNotification> _pendingNotifications;
		std::weak_ptr<IActionDispatcher> _dispatcher;
		std::mutex _lock;

#pragma endregion
	};
}

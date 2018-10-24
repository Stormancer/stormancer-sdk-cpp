#pragma once
#include "Stormancer/headers.h"
#include "Stormancer/RPC/RpcService.h"
#include "InAppNotification.h"

namespace Stormancer
{
	class InAppNotificationService
	{
	public:

#pragma region public_methods

		InAppNotificationService(Scene* scene);

		void registerNotificationsCallback(const std::function<void(InAppNotification)>& callback);

		void notificationReceived(const InAppNotification& notification);

		pplx::task<void> acknowledgeNotification(const std::string& notificationId);

#pragma endregion

	private:

#pragma region
	
		Scene* _scene;
		std::shared_ptr<RpcService> _rpcService;
		std::function<void(const InAppNotification&)> _callback;
		std::queue<InAppNotification> _pendingNotifications;
		std::weak_ptr<IActionDispatcher> _dispatcher;
		std::mutex _lock;

#pragma endregion
	};
}

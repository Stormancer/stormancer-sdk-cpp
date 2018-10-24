#include "Stormancer/stdafx.h"
#include "InAppNotificationService.h"

namespace Stormancer
{
	InAppNotificationService::InAppNotificationService(Scene* scene)
	{
		_scene = scene;
		if (scene)
		{
			_rpcService = scene->dependencyResolver().lock()->resolve<RpcService>();
			_dispatcher = scene->dependencyResolver().lock()->resolve<IActionDispatcher>();

			scene->addRoute("inappnotification.push", [this](Packetisp_ptr packet) {
				Serializer serializer;
				InAppNotification notification = serializer.deserializeOne<InAppNotification>(packet->stream);
				notificationReceived(notification);
			});
		}
	}

	void InAppNotificationService::registerNotificationsCallback(const std::function<void(InAppNotification)>& callback)
	{
		std::lock_guard<std::mutex> lg(_lock);

		_callback = callback;

		if (_callback)
		{
			while (!_pendingNotifications.empty())
			{
				InAppNotification notification = _pendingNotifications.front();
				_pendingNotifications.pop();
				_callback(notification);
			}
		}
	}

	void InAppNotificationService::notificationReceived(const InAppNotification& notification)
	{
		// lock_guard scope
		{
			std::lock_guard<std::mutex> lg(_lock);

			if (_callback)
			{
				auto dispatcher = _dispatcher.lock();
				if (dispatcher)
				{
					pplx::create_task([this, notification]() {
						_callback(notification);
					}, pplx::task_options(dispatcher));
				}
				else
				{
					_callback(notification);
				}
			}
			else
			{
				_pendingNotifications.push(notification);
			}
		}

		if (notification.Acknowledgment == InAppNotificationAcknowledgment::OnReceive)
		{
			acknowledgeNotification(notification.id);
		}

	}

	pplx::task<void> InAppNotificationService::acknowledgeNotification(const std::string& notificationId)
	{
		return _rpcService->rpc<void, std::string>("inappnotification.acknowledgenotification", notificationId);
	}
}

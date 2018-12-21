#if defined(STORMANCER_CUSTOM_PCH)
#include STORMANCER_CUSTOM_PCH
#endif
#include "Members/FriendsService.h"
#include "stormancer/headers.h"
#include "Authentication/AuthenticationService.h"
#include "stormancer/Scene.h"

namespace Stormancer
{
	FriendsService::FriendsService(std::shared_ptr<Scene> scene, std::shared_ptr<ILogger> logger) :
		_scene(scene)
		, _logger(logger)
	{
		scene->addRoute("friends.notification", [this](Packetisp_ptr packet) {
			auto update = packet->readObject<FriendListUpdateDto>();
			this->onFriendNotification(update);
		});
	}

	pplx::task<void> FriendsService::inviteFriend(std::string userId)
	{
		auto scene = _scene.lock();
		if (!scene)
		{
			return pplx::task_from_exception<void>(std::runtime_error("scene deleted"));
		}

		return scene->dependencyResolver()->resolve<RpcService>()->rpc<void, std::string>("friends.invitefriend", userId);
	}

	pplx::task<void> FriendsService::answerFriendInvitation(std::string originId, bool accept)
	{
		auto scene = _scene.lock();
		if (!scene)
		{
			return pplx::task_from_exception<void>(std::runtime_error("scene deleted"));
		}

		return scene->dependencyResolver()->resolve<RpcService>()->rpc<void, std::string, bool>("friends.acceptfriendinvitation", originId, accept);
	}

	pplx::task<void> FriendsService::removeFriend(std::string userId)
	{
		auto scene = _scene.lock();
		if (!scene)
		{
			return pplx::task_from_exception<void>(std::runtime_error("scene deleted"));
		}

		return scene->dependencyResolver()->resolve<RpcService>()->rpc<void, std::string>("friends.removefriend", userId);
	}

	pplx::task<void> FriendsService::setStatus(FriendListStatusConfig status, std::string details)
	{
		auto scene = _scene.lock();
		if (!scene)
		{
			return pplx::task_from_exception<void>(std::runtime_error("scene deleted"));
		}

		return scene->dependencyResolver()->resolve<RpcService>()->rpc<void, FriendListStatusConfig, std::string >("friends.setstatus", status, details);
	}

	void FriendsService::onFriendNotification(const FriendListUpdateDto &update)
	{
		auto operation = update.operation;
		if (operation == "remove")
		{
			onFriendRemove(update);
		}
		else if (operation == "update")
		{
			onFriendUpdate(update);
		}
		else if (operation == "add")
		{
			onFriendAdd(update);
		}
		else if (operation == "update.status")
		{
			onFriendUpdateStatus(update);
		}
		else
		{
			_logger->log(LogLevel::Error, "friends", "Unknown friends operation: " + update.operation);
		}

	}

	void FriendsService::onFriendAdd(const FriendListUpdateDto & update)
	{
		auto fr = std::make_shared<Friend>(update.data);
		friends[update.itemId] = fr;
		FriendListUpdateEvent ev;
		ev.op = FriendListUpdateOperation::Add;
		ev.value = fr;
		friendListChanged(ev);
	}

	void FriendsService::onFriendUpdate(const FriendListUpdateDto & update)
	{
		auto fr = friends[update.itemId];
		if (fr)
		{
			fr->status = update.data.status;
			fr->details = update.data.details;
			fr->lastConnected = update.data.lastConnected;
			fr->userId = update.data.userId;


			FriendListUpdateEvent ev;
			ev.op = FriendListUpdateOperation::Update;
			ev.value = fr;
			friendListChanged(ev);
		}
	}

	void FriendsService::onFriendUpdateStatus(const FriendListUpdateDto & update)
	{
		auto fr = friends[update.itemId];
		if (fr)
		{
			fr->status = update.data.status;
			FriendListUpdateEvent ev;
			ev.op = FriendListUpdateOperation::Update;
			ev.value = fr;
			friendListChanged(ev);
		}
	}

	void FriendsService::onFriendRemove(const FriendListUpdateDto & update)
	{
		auto fr = friends[update.itemId];
		if (fr)
		{
			friends.erase(update.itemId);
			FriendListUpdateEvent ev;
			ev.op = FriendListUpdateOperation::Add;
			ev.value = fr;
			friendListChanged(ev);
		}
	}
}
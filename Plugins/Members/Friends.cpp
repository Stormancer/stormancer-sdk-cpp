#include "stormancer/headers.h"
#include "Friends.h"
#include "Authentication/AuthenticationService.h"
#include "stormancer/Scene.h"

Stormancer::FriendsService::FriendsService(Scene_ptr scene, std::shared_ptr<ILogger> logger) :
	_scene(scene)
	, _logger(logger)
{
	scene->addRoute("friends.notification", [this](Packetisp_ptr packet) {
		auto update = packet->readObject<FriendListUpdateDto>();
		this->onFriendNotification(update);
	});
}

pplx::task<void> Stormancer::FriendsService::inviteFriend(std::string userId)
{
	auto scene = _scene.lock();
	if (!scene)
	{
		return pplx::task_from_exception<void>(std::runtime_error("scene deleted"));
	}

	return scene->dependencyResolver().lock()->resolve<RpcService>()->rpc<void, std::string>("friends.invitefriend", userId);
}

pplx::task<void> Stormancer::FriendsService::answerFriendInvitation(std::string originId, bool accept)
{
	auto scene = _scene.lock();
	if (!scene)
	{
		return pplx::task_from_exception<void>(std::runtime_error("scene deleted"));
	}

	return scene->dependencyResolver().lock()->resolve<RpcService>()->rpc<void, std::string, bool>("friends.acceptfriendinvitation", originId, accept);
}

pplx::task<void> Stormancer::FriendsService::removeFriend(std::string userId)
{
	auto scene = _scene.lock();
	if (!scene)
	{
		return pplx::task_from_exception<void>(std::runtime_error("scene deleted"));
	}

	return scene->dependencyResolver().lock()->resolve<RpcService>()->rpc<void, std::string>("friends.removefriend", userId);
}

pplx::task<void> Stormancer::FriendsService::setStatus(FriendListStatusConfig status, std::string details)
{
	auto scene = _scene.lock();
	if (!scene)
	{
		return pplx::task_from_exception<void>(std::runtime_error("scene deleted"));
	}

	return scene->dependencyResolver().lock()->resolve<RpcService>()->rpc<void, FriendListStatusConfig, std::string >("friends.setstatus", status, details);
}

void Stormancer::FriendsService::onFriendNotification(const FriendListUpdateDto &update)
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

void Stormancer::FriendsService::onFriendAdd(const FriendListUpdateDto & update)
{
	auto fr = std::make_shared<Friend>(update.data);
	friends[update.itemId] = fr;
	FriendListUpdateEvent ev;
	ev.op = FriendListUpdateOperation::Add;
	ev.value = fr;
	friendListChanged(ev);
}

void Stormancer::FriendsService::onFriendUpdate(const FriendListUpdateDto & update)
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

void Stormancer::FriendsService::onFriendUpdateStatus(const FriendListUpdateDto & update)
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

void Stormancer::FriendsService::onFriendRemove(const FriendListUpdateDto & update)
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


namespace Stormancer
{
	Friends::Friends(std::weak_ptr<AuthenticationService> auth) :
		ClientAPI(auth)
	{
	}

	pplx::task<std::unordered_map<std::string, std::shared_ptr<Friend>>> Friends::friends()
	{
		return getFriendService().then([](std::shared_ptr<FriendsService> s) {return s->friends; });
	}

	pplx::task<void> Friends::inviteFriend(std::string userId)
	{
		return getFriendService().then([userId](std::shared_ptr<FriendsService> s) {return s->inviteFriend(userId); });
	}

	pplx::task<void> Friends::answerFriendInvitation(std::string originId, bool accept)
	{
		return getFriendService().then([originId, accept](std::shared_ptr<FriendsService> s) {return s->answerFriendInvitation(originId, accept); });
	}

	pplx::task<void> Friends::removeFriend(std::string userId)
	{
		return getFriendService().then([userId](std::shared_ptr<FriendsService> s) {return s->removeFriend(userId); });
	}

	pplx::task<void> Friends::setStatus(FriendListStatusConfig status, std::string details)
	{
		return getFriendService().then([status, details](std::shared_ptr<FriendsService> s) {return s->setStatus(status, details); });
	}

	pplx::task<std::shared_ptr<FriendsService>> Friends::getFriendService()
	{

		return this->getService<FriendsService>("stormancer.friends",
			[](auto that, auto friends, auto scene){

			auto wThat = that->weak_from_this();
			that->_friendListChangedSubscription = friends->friendListChanged.subscribe([wThat](FriendListUpdateEvent ev)
			{
				auto that = wThat.lock();
				if (!that)
				{
					throw std::runtime_error("destroyed");
				}

				that->friendListChanged(ev);
			});

		},
			[](auto that, auto scene) {
		
			that->_friendListChangedSubscription = nullptr;
		});

	}


}
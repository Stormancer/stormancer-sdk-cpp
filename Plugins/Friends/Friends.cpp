#include "stdafx.h"
#include "Friends.h"

Stormancer::FriendsService::FriendsService(std::shared_ptr<Scene> scene, std::shared_ptr<ILogger> logger)
	: friendListChanged([](std::shared_ptr<Friend> f, FriendListUpdateOperation /*op*/) {})
	, _scene(scene)
	, _logger(logger)
{
	scene->addRoute("friends.notification", [this](Packetisp_ptr packet) {
		auto update = packet->readObject<FriendListUpdateDto>();
		this->onFriendNotification(update);
	});
}

pplx::task<void> Stormancer::FriendsService::inviteFriend(std::string userId)
{
	return _scene->dependencyResolver()->resolve<RpcService>()->rpc<void, std::string>("friends.invitefriend", userId);
}

pplx::task<void> Stormancer::FriendsService::answerFriendInvitation(std::string originId, bool accept)
{
	return _scene->dependencyResolver()->resolve<RpcService>()->rpc<void, std::string, bool>("friends.acceptfriendinvitation", originId, accept);
}

pplx::task<void> Stormancer::FriendsService::removeFriend(std::string userId)
{
	return _scene->dependencyResolver()->resolve<RpcService>()->rpc<void, std::string>("friends.removefriend", userId);
}

pplx::task<void> Stormancer::FriendsService::setStatus(FriendListStatusConfig status, std::string details)
{
	return _scene->dependencyResolver()->resolve<RpcService>()->rpc<void, FriendListStatusConfig, std::string >("friends.setstatus", status, details);
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
	friendListChanged(fr, FriendListUpdateOperation::Add);
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

		friendListChanged(fr, FriendListUpdateOperation::Update);

	}
}

void Stormancer::FriendsService::onFriendUpdateStatus(const FriendListUpdateDto & update)
{
	auto fr = friends[update.itemId];
	if (fr)
	{
		fr->status = update.data.status;
		friendListChanged(fr, FriendListUpdateOperation::Update);
	}
}

void Stormancer::FriendsService::onFriendRemove(const FriendListUpdateDto & update)
{
	auto fr = friends[update.itemId];
	if (fr)
	{
		friends.erase(update.itemId);
		friendListChanged(fr, FriendListUpdateOperation::Remove);
	}
}

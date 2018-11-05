#pragma once

#include "stormancer/stormancer.h"
#include "stormancer/Event.h"
#include "Core/ClientAPI.h"

namespace Stormancer
{
	enum class FriendStatus : int
	{
		Online = 0,
		Away = 1,
		InvitationPending = 2,
		Disconnected = 3
	};

	enum class FriendListStatusConfig
	{
		Online = 0,
		Invisible = 1,
		Away = 2
	};

	struct Friend
	{
		std::string userId;
		std::string details;
		uint64 lastConnected;
		FriendStatus status;
		std::vector<std::string> tags;

		MSGPACK_DEFINE(userId, details, lastConnected, status, tags)
	};

	struct FriendListUpdateDto
	{
		std::string itemId;
		std::string operation;
		Friend data;

		MSGPACK_DEFINE(itemId, operation, data)
	};

	enum class FriendListUpdateOperation
	{
		Add = 0,
		Remove = 1,
		Update = 2,
	};

	struct FriendListUpdateEvent
	{
		FriendListUpdateOperation op;
		std::shared_ptr<Friend> value;
	};

	class FriendsService
	{
	public:

		FriendsService(Scene_ptr scene, std::shared_ptr<ILogger> logger);
		Event<FriendListUpdateEvent> friendListChanged;
		std::unordered_map<std::string,std::shared_ptr<Friend>> friends;
		pplx::task<void> inviteFriend(std::string userId);
		pplx::task<void> answerFriendInvitation(std::string originId, bool accept = true);
		pplx::task<void> removeFriend(std::string userId);
		pplx::task<void> setStatus(FriendListStatusConfig status, std::string details);

	private:

		void onFriendNotification(const FriendListUpdateDto &update);
		void onFriendAdd(const FriendListUpdateDto &update);
		void onFriendUpdate(const FriendListUpdateDto &update);
		void onFriendUpdateStatus(const FriendListUpdateDto &update);
		void onFriendRemove(const FriendListUpdateDto &update);

		std::weak_ptr<Scene> _scene;
		ILogger_ptr _logger;
	};

	class Friends: public ClientAPI<Friends>
	{
	public:

		Friends(std::weak_ptr<AuthenticationService> auth);
		Event<FriendListUpdateEvent> friendListChanged;
		pplx::task<std::unordered_map<std::string, std::shared_ptr<Friend>>> friends();
		pplx::task<void> inviteFriend(std::string userId);
		pplx::task<void> answerFriendInvitation(std::string originId, bool accept = true);
		pplx::task<void> removeFriend(std::string userId);
		pplx::task<void> setStatus(FriendListStatusConfig status, std::string details);

	private:
		
		pplx::task<std::shared_ptr<FriendsService>> getFriendService();
		Event<FriendListUpdateEvent>::Subscription _friendListChangedSubscription;
	};
}

MSGPACK_ADD_ENUM(Stormancer::FriendStatus);
MSGPACK_ADD_ENUM(Stormancer::FriendListStatusConfig);

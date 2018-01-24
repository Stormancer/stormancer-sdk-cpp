#pragma once
#include "stormancer.h"

namespace Stormancer
{
	enum class FriendStatus :int
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

		MSGPACK_DEFINE(userId, details, lastConnected, status)
	};

	struct FriendListUpdateDto
	{
		std::string itemId;
		std::string operation;
		Friend data;

		MSGPACK_DEFINE(itemId,operation,data)
	};

	enum class FriendListUpdateOperation
	{
		Add,
		Remove,
		Update
	};

	class FriendsService
	{
	public:
		FriendsService(std::shared_ptr<Scene> scene, std::shared_ptr<ILogger> logger);

		std::function<void(std::shared_ptr<Friend>, FriendListUpdateOperation)> friendListChanged;

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

		std::shared_ptr<Scene> _scene;
		std::shared_ptr<ILogger> _logger;
	};
	
		
}

MSGPACK_ADD_ENUM(Stormancer::FriendStatus);
MSGPACK_ADD_ENUM(Stormancer::FriendListStatusConfig);
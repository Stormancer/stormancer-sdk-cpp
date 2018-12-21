#pragma once
#include "stormancer/headers.h"

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
}

MSGPACK_ADD_ENUM(Stormancer::FriendStatus);
MSGPACK_ADD_ENUM(Stormancer::FriendListStatusConfig);

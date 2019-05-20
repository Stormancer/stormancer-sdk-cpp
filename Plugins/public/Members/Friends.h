#pragma once

#include "stormancer/Event.h"
#include "stormancer/Tasks.h"
#include "Members/FriendsModels.h"
#include <unordered_map>
#include <functional>
#include <string>
#include <memory>

namespace Stormancer
{
	class Friends
	{
	public:
		virtual ~Friends() {}
		virtual pplx::task<std::unordered_map<std::string, std::shared_ptr<Friend>>> friends() = 0;
		virtual pplx::task<void> inviteFriend(std::string userId) = 0;
		virtual pplx::task<void> answerFriendInvitation(std::string originId, bool accept = true) = 0;
		virtual pplx::task<void> removeFriend(std::string userId) = 0;
		virtual pplx::task<void> setStatus(FriendListStatusConfig status, std::string details) = 0;

		virtual Event<FriendListUpdateEvent>::Subscription subscribeFriendListUpdateEvent(std::function<void(FriendListUpdateEvent)> callback) = 0;
	};
}
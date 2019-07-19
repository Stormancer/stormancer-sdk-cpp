#pragma once

#include "stormancer/Event.h"
#include "stormancer/ClientAPI.h"
#include "Members/Friends.h"
#include "FriendsModels.h"

namespace Stormancer
{
	class FriendsService;

	class Friends_Impl : public ClientAPI<Friends_Impl>, public Friends
	{
	public:

		Friends_Impl(std::weak_ptr<AuthenticationService> auth);
		
		pplx::task<std::unordered_map<std::string, std::shared_ptr<Friend>>> friends() override;
		pplx::task<void> inviteFriend(std::string userId) override;
		pplx::task<void> answerFriendInvitation(std::string originId, bool accept = true) override;
		pplx::task<void> removeFriend(std::string userId) override;
		pplx::task<void> setStatus(FriendListStatusConfig status, std::string details) override;

		Event<FriendListUpdateEvent>::Subscription subscribeFriendListUpdateEvent(std::function<void(FriendListUpdateEvent)> callback) override;
		Event<FriendListUpdateEvent> friendListChanged;
	private:
		
		pplx::task<std::shared_ptr<FriendsService>> getFriendService();
		Event<FriendListUpdateEvent>::Subscription _friendListChangedSubscription;
	};
}
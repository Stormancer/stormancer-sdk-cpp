#if defined(STORMANCER_CUSTOM_PCH)
#include STORMANCER_CUSTOM_PCH
#endif
#include "Friends_Impl.h"
#include "FriendsService.h"
#include "Authentication/AuthenticationService.h"
#include "stormancer/Scene.h"

namespace Stormancer
{
	Friends_Impl::Friends_Impl(std::weak_ptr<AuthenticationService> auth) :
		ClientAPI(auth)
	{
	}

	pplx::task<std::unordered_map<std::string, std::shared_ptr<Friend>>> Friends_Impl::friends()
	{
		return getFriendService().then([](std::shared_ptr<FriendsService> s) {return s->friends; });
	}

	pplx::task<void> Friends_Impl::inviteFriend(std::string userId)
	{
		return getFriendService().then([userId](std::shared_ptr<FriendsService> s) {return s->inviteFriend(userId); });
	}

	pplx::task<void> Friends_Impl::answerFriendInvitation(std::string originId, bool accept)
	{
		return getFriendService().then([originId, accept](std::shared_ptr<FriendsService> s) {return s->answerFriendInvitation(originId, accept); });
	}

	pplx::task<void> Friends_Impl::removeFriend(std::string userId)
	{
		return getFriendService().then([userId](std::shared_ptr<FriendsService> s) {return s->removeFriend(userId); });
	}

	pplx::task<void> Friends_Impl::setStatus(FriendListStatusConfig status, std::string details)
	{
		return getFriendService().then([status, details](std::shared_ptr<FriendsService> s) {return s->setStatus(status, details); });
	}

	Event<FriendListUpdateEvent>::Subscription Friends_Impl::subscribeFriendListUpdateEvent(std::function<void(FriendListUpdateEvent)> callback)
	{
		return friendListChanged.subscribe(callback);;
	}

	pplx::task<std::shared_ptr<FriendsService>> Friends_Impl::getFriendService()
	{
		auto initializer = [](auto that, auto friends, auto scene) {
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
		};

		auto cleanup = [](auto that, auto type) {
			that->_friendListChangedSubscription = nullptr;
		};

		return this->getService<FriendsService>("stormancer.friends", initializer, cleanup);
	}
}

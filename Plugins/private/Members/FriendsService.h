#pragma once

#include "stormancer/Event.h"
#include "stormancer/Tasks.h"
#include "FriendsModels.h"
#include <unordered_map>
#include <string>
#include <memory>

namespace Stormancer
{
	///
	/// Forward declare
	class Scene; 
	class FriendsService;
	class ILogger;

	class FriendsService
	{
	public:

		FriendsService(std::shared_ptr<Scene> scene, std::shared_ptr<ILogger> logger);
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
		std::shared_ptr<ILogger> _logger;
	};
}
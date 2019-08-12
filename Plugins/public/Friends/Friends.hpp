#pragma once

#include "stormancer/StormancerTypes.h"
#include "stormancer/msgpack_define.h"
#include "stormancer/ClientAPI.h"
#include "stormancer/Event.h"
#include "stormancer/Tasks.h"
#include "stormancer/IPlugin.h"
#include "stormancer/Scene.h"
#include "stormancer/Utilities/PointerUtilities.h"
#include "Authentication/AuthenticationService.h"
#include <unordered_map>
#include <functional>
#include <string>
#include <vector>
#include <memory>

namespace Stormancer
{
	namespace Friends
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

		class IFriendsEventHandler
		{
		public:

			virtual void getFriends(std::unordered_map<std::string, std::shared_ptr<Friend>>& friends) = 0;
		};

		class Friends
		{
		public:
			virtual ~Friends() = default;
			virtual pplx::task<std::unordered_map<std::string, std::shared_ptr<Friend>>> friends() = 0;
			virtual pplx::task<void> inviteFriend(std::string userId) = 0;
			virtual pplx::task<void> answerFriendInvitation(std::string originId, bool accept = true) = 0;
			virtual pplx::task<void> removeFriend(std::string userId) = 0;
			virtual pplx::task<void> setStatus(FriendListStatusConfig status, std::string details) = 0;

			virtual Event<FriendListUpdateEvent>::Subscription subscribeFriendListUpdateEvent(std::function<void(FriendListUpdateEvent)> callback) = 0;
		};

		class FriendsService
		{
		public:

			FriendsService(std::shared_ptr<Scene> scene, std::shared_ptr<ILogger> logger)
				: _scene(scene)
				, _logger(logger)
			{
				scene->addRoute("friends.notification", [this](Packetisp_ptr packet)
				{
					auto update = packet->readObject<FriendListUpdateDto>();
					this->onFriendNotification(update);
				});
			}

			Event<FriendListUpdateEvent> friendListChanged;
			std::unordered_map<std::string, std::shared_ptr<Friend>> friends;

			pplx::task<void> inviteFriend(std::string userId)
			{
				auto scene = _scene.lock();
				if (!scene)
				{
					return pplx::task_from_exception<void>(std::runtime_error("scene deleted"));
				}

				return scene->dependencyResolver().resolve<RpcService>()->rpc<void, std::string>("friends.invitefriend", userId);
			}

			pplx::task<void> answerFriendInvitation(std::string originId, bool accept = true)
			{
				auto scene = _scene.lock();
				if (!scene)
				{
					return pplx::task_from_exception<void>(std::runtime_error("scene deleted"));
				}

				return scene->dependencyResolver().resolve<RpcService>()->rpc<void, std::string, bool>("friends.acceptfriendinvitation", originId, accept);
			}

			pplx::task<void> removeFriend(std::string userId)
			{
				auto scene = _scene.lock();
				if (!scene)
				{
					return pplx::task_from_exception<void>(std::runtime_error("scene deleted"));
				}

				return scene->dependencyResolver().resolve<RpcService>()->rpc<void, std::string>("friends.removefriend", userId);
			}

			pplx::task<void> setStatus(FriendListStatusConfig status, std::string details)
			{
				auto scene = _scene.lock();
				if (!scene)
				{
					return pplx::task_from_exception<void>(std::runtime_error("scene deleted"));
				}

				return scene->dependencyResolver().resolve<RpcService>()->rpc<void, FriendListStatusConfig, std::string >("friends.setstatus", status, details);
			}

		private:

			void onFriendNotification(const FriendListUpdateDto &update)
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

			void onFriendAdd(const FriendListUpdateDto &update)
			{
				auto fr = std::make_shared<Friend>(update.data);
				friends[update.itemId] = fr;
				FriendListUpdateEvent ev;
				ev.op = FriendListUpdateOperation::Add;
				ev.value = fr;
				friendListChanged(ev);
			}

			void onFriendUpdate(const FriendListUpdateDto &update)
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

			void onFriendUpdateStatus(const FriendListUpdateDto &update)
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

			void onFriendRemove(const FriendListUpdateDto &update)
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

			std::weak_ptr<Scene> _scene;
			std::shared_ptr<ILogger> _logger;
		};

		class Friends_Impl : public ClientAPI<Friends_Impl>, public Friends
		{
		public:

			Friends_Impl(std::weak_ptr<AuthenticationService> auth, std::vector<std::shared_ptr<IFriendsEventHandler>> friendsEventHandlers)
				: ClientAPI(auth)
				, _friendsEventHandlers(friendsEventHandlers)
			{
			}

			pplx::task<std::unordered_map<std::string, std::shared_ptr<Friend>>> friends() override
			{
				auto wFriends = STORM_WEAK_FROM_THIS();
				return getFriendService()
					.then([wFriends](std::shared_ptr<FriendsService> s)
				{
					auto myFriends = s->friends;
					if (auto friends = wFriends.lock())
					{
						for (auto& handler : friends->_friendsEventHandlers)
						{
							handler->getFriends(myFriends);
						}
					}
					return myFriends;
				});
			}

			pplx::task<void> inviteFriend(std::string userId) override
			{
				return getFriendService().then([userId](std::shared_ptr<FriendsService> s) {return s->inviteFriend(userId); });
			}

			pplx::task<void> answerFriendInvitation(std::string originId, bool accept = true) override
			{
				return getFriendService().then([originId, accept](std::shared_ptr<FriendsService> s) {return s->answerFriendInvitation(originId, accept); });
			}

			pplx::task<void> removeFriend(std::string userId) override
			{
				return getFriendService().then([userId](std::shared_ptr<FriendsService> s) {return s->removeFriend(userId); });
			}

			pplx::task<void> setStatus(FriendListStatusConfig status, std::string details) override
			{
				return getFriendService().then([status, details](std::shared_ptr<FriendsService> s) {return s->setStatus(status, details); });
			}

			Subscription subscribeFriendListUpdateEvent(std::function<void(FriendListUpdateEvent)> callback) override
			{
				return friendListChanged.subscribe(callback);
			}

			Event<FriendListUpdateEvent> friendListChanged;

		private:

			pplx::task<std::shared_ptr<FriendsService>> getFriendService()
			{
				auto initializer = [](auto that, auto friends, auto scene)
				{
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

			Event<FriendListUpdateEvent>::Subscription _friendListChangedSubscription;
			std::vector<std::shared_ptr<IFriendsEventHandler>> _friendsEventHandlers;
		};

		class FriendsPlugin : public IPlugin
		{
		public:

			void registerSceneDependencies(ContainerBuilder& builder, std::shared_ptr<Scene> scene) override
			{
				if (scene)
				{
					auto name = scene->getHostMetadata("stormancer.friends");
					if (name.length() > 0)
					{
						builder.registerDependency<FriendsService, Scene, ILogger>().singleInstance();
					}
				}
			}

			void registerClientDependencies(ContainerBuilder& builder) override
			{
				builder.registerDependency<Friends_Impl, AuthenticationService, ContainerBuilder::All<IFriendsEventHandler>>().as<Friends>().singleInstance();
			}
		};
	}
}

MSGPACK_ADD_ENUM(Stormancer::Friends::FriendStatus);
MSGPACK_ADD_ENUM(Stormancer::Friends::FriendListStatusConfig);

#pragma once

#include "Users/ClientAPI.hpp"
#include "Users/Users.hpp"
#include "stormancer/StormancerTypes.h"
#include "stormancer/msgpack_define.h"
#include "stormancer/Event.h"
#include "stormancer/Tasks.h"
#include "stormancer/IPlugin.h"
#include "stormancer/Scene.h"
#include "stormancer/Utilities/PointerUtilities.h"
#include <unordered_map>
#include <functional>
#include <string>
#include <vector>
#include <memory>

namespace Stormancer
{
	namespace Friends
	{
		enum class FriendStatus
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

		enum class FriendListUpdateOperationInternal
		{
			Add = 0,
			Remove = 1,
			Update = 2,
			UpdateStatus = 3
		};

		struct FriendListUpdateDto
		{
			std::string itemId;
			FriendListUpdateOperationInternal operation;
			Friend data;

			MSGPACK_DEFINE(itemId, operation, data)
		};

		enum class FriendListUpdateOperation
		{
			Add = 0,
			Remove = 1,
			Update = 2
		};

		/// <summary>
		/// Represents a friend list update event.
		/// </summary>
		struct FriendListUpdatedEvent
		{
			FriendListUpdateOperation operation;
			std::shared_ptr<Friend> value;
		};

		/// <summary>
		/// Abstract class representing the contract for friends event handlers
		/// </summary>
		class IFriendsEventHandler
		{
		public:

			/// <summary>
			/// Function called when the friendlist is loaded initially.
			/// </summary>
			/// <param name="friends"></param>
			virtual void getFriends(std::unordered_map<std::string, std::shared_ptr<Friend>>& friends) = 0;
		};

		/// <summary>
		/// Represents the result of a get friends operation
		/// </summary>
		struct FriendsResult
		{
			/// <summary>
			/// true if the friends list is ready, false if not.
			/// </summary>
			/// <remarks>
			/// If false, friends is empty.
			/// </remarks>
			bool isReady = false;

			/// <summary>
			/// List of friends indexed by id
			/// </summary>
			/// <remarks>
			/// If the friend list is not ready (isReady = false), the list is empty.
			/// </remarks>
			std::unordered_map<std::string, Friend> friends;
		};

		/// <summary>
		/// Friends API
		/// </summary>
		/// <remarks>
		/// 
		/// </remarks>
		class Friends
		{
		public:
			virtual ~Friends() = default;
			/// <summary>
			/// Gets the list of friends.
			/// </summary>
			/// <remarks>
			/// The Friends class maintains locally a list of friends and fires event if it is updated. 
			/// </remarks>
			/// <returns>A FriendsResult object</returns>
			virtual FriendsResult friends() = 0;

			/// <summary>
			/// Gets a boolean value indicating whether the friend list is loaded or not.
			/// </summary>
			/// <returns>True if the friend list is loaded, false otherwise.</returns>
			virtual bool isLoaded() = 0;

			/// <summary>
			/// Invites an user to the friend list.
			/// </summary>
			/// <param name="userId">Id of the user to invite.</param>
			/// <returns>A task that completes when the server acknowledged the invitation request.</returns>
			virtual pplx::task<void> inviteFriend(std::string userId) = 0;

			/// <summary>
			/// Answers a friend invitation.
			/// </summary>
			/// <param name="originId">Id of the user that sent the friend invitation.</param>
			/// <param name="accept">If true, accepts the invitation. If false, refuse.</param>
			/// <returns>A task that completes when the server acknowledged the request.</returns>
			virtual pplx::task<void> answerFriendInvitation(std::string originId, bool accept = true) = 0;

			/// <summary>
			/// Removes a friend from the list.
			/// </summary>
			/// <param name="userId">Id of the user to remove.</param>
			/// <returns>A task that completes when the server acknowledged the request.</returns>
			virtual pplx::task<void> removeFriend(std::string userId) = 0;

			/// <summary>
			/// Updates the current user's status in the friend system.
			/// </summary>
			/// <remarks>
			/// The details argument is made available to server side plugins that react to player status updates to perform additional tasks, for instance:
			/// Modifying or augmenting the status before save, performing operation on other systems like guilds or profiles, etc...
			/// </remarks>
			/// <param name="status">The updated status information.</param>
			/// <param name="details">Optional data to be processed by the server.</param>
			/// <returns></returns>
			virtual pplx::task<void> setStatus(FriendListStatusConfig status, std::string details) = 0;

			/// <summary>
			/// Event fired whenever the friend list content changes.
			/// </summary>
			/// <param name="callback">Callback called when the event is fired.</param>
			/// <returns>An object that controls the lifetime of the event subscription. If all copies of this object are destroyed, the callbeck is automatically unregistered.</returns>
			virtual Event<FriendListUpdatedEvent>::Subscription subscribeFriendListUpdatedEvent(std::function<void(FriendListUpdatedEvent)> callback) = 0;

			/// <summary>
			/// Ask the friend list for a full refresh. This should be called only in platform events when users are added or removed from the user friend list.
			/// </summary>
			/// <returns>A task which terminate when the server has returned the friend list and the local plugin processed the changes localy.</returns>
			virtual pplx::task<void> refresh() = 0;
		};

		namespace details
		{
			class FriendsService : public std::enable_shared_from_this<FriendsService>
			{
			public:

				FriendsService(std::shared_ptr<Scene> scene, std::shared_ptr<ILogger> logger)
					: _scene(scene)
					, _logger(logger)
					, _rpcService(scene->dependencyResolver().resolve<RpcService>())
				{
				}

				Event<FriendListUpdatedEvent> friendListChanged;
				std::unordered_map<std::string, std::shared_ptr<Friend>> friends;

				void initialize()
				{
					std::weak_ptr<FriendsService> wFriendsService = this->shared_from_this();
					_scene.lock()->addRoute<std::vector<FriendListUpdateDto>>("friends.notification", [wFriendsService](std::vector<FriendListUpdateDto> friendUpdates)
					{
						if (auto friendsService = wFriendsService.lock())
						{
							for (auto friendUpdate : friendUpdates)
							{
								friendsService->onFriendNotification(friendUpdate);
							}
							friendsService->_isLoaded = true;
						}
					});
				}

				pplx::task<void> inviteFriend(std::string userId)
				{
					return _rpcService->rpc<void, std::string>("friends.invitefriend", userId);
				}

				pplx::task<void> answerFriendInvitation(std::string originId, bool accept = true)
				{
					return _rpcService->rpc<void, std::string, bool>("friends.acceptfriendinvitation", originId, accept);
				}

				pplx::task<void> removeFriend(std::string userId)
				{
					return _rpcService->rpc<void, std::string>("friends.removefriend", userId);
				}

				pplx::task<void> setStatus(FriendListStatusConfig status, std::string details)
				{
					return _rpcService->rpc<void, FriendListStatusConfig, std::string >("friends.setstatus", status, details);
				}

				pplx::task<void> refresh()
				{
					std::weak_ptr<FriendsService> wFriendsService = this->shared_from_this();
					return _rpcService->rpc<std::unordered_map<std::string, Friend>>("friends.refresh")
						.then([wFriendsService](std::unordered_map<std::string, Friend> newFriends)
					{
						if (auto friendsService = wFriendsService.lock())
						{
							auto& friends = friendsService->friends;
							auto oldFriends = friends;

							for (auto oldFriendIt : oldFriends)
							{
								if (newFriends.find(oldFriendIt.first) == newFriends.end())
								{
									// REMOVE
									friends.erase(oldFriendIt.first);
									friendsService->friendListChanged(FriendListUpdatedEvent{ FriendListUpdateOperation::Remove, oldFriendIt.second });
								}
							}

							for (auto newFriend : newFriends)
							{
								auto friendIt = oldFriends.find(newFriend.first);
								if (friendIt == oldFriends.end())
								{
									// ADD
									auto fr = friends[newFriend.first] = std::make_shared<Friend>(newFriend.second);
									friendsService->friendListChanged(FriendListUpdatedEvent{ FriendListUpdateOperation::Add, fr });
								}
								else
								{
									// UPDATE
									auto fr = friendIt->second;
									if (fr)
									{
										fr->status = newFriend.second.status;
										fr->details = newFriend.second.details;
										fr->lastConnected = newFriend.second.lastConnected;
										fr->userId = newFriend.second.userId;
										friendsService->friendListChanged(FriendListUpdatedEvent{ FriendListUpdateOperation::Update, fr });
									}
								}
							}
						}
					});
				}

				bool isLoaded()
				{
					return _isLoaded;
				}

			private:

				void onFriendNotification(const FriendListUpdateDto& update)
				{
					switch (update.operation)
					{
					case FriendListUpdateOperationInternal::Remove:
						onFriendRemove(update);
						break;
					case FriendListUpdateOperationInternal::Update:
						onFriendUpdate(update);
						break;
					case FriendListUpdateOperationInternal::Add:
						onFriendAdd(update);
						break;
					case FriendListUpdateOperationInternal::UpdateStatus:
						onFriendUpdateStatus(update);
						break;
					default:
						_logger->log(LogLevel::Error, "friends", "Unknown friends operation: " + std::to_string((int)update.operation));
						break;
					}
				}

				void onFriendAdd(const FriendListUpdateDto& update)
				{
					auto fr = std::make_shared<Friend>(update.data);
					friends[update.itemId] = fr;
					friendListChanged(FriendListUpdatedEvent{ FriendListUpdateOperation::Add, fr });
				}

				void onFriendUpdate(const FriendListUpdateDto& update)
				{
					auto friendIt = friends.find(update.itemId);
					if (friendIt != friends.end())
					{
						auto fr = friendIt->second;
						if (fr)
						{
							fr->status = update.data.status;
							fr->details = update.data.details;
							fr->lastConnected = update.data.lastConnected;
							fr->userId = update.data.userId;
							friendListChanged(FriendListUpdatedEvent{ FriendListUpdateOperation::Update, fr });
						}
					}
				}

				void onFriendUpdateStatus(const FriendListUpdateDto& update)
				{
					auto friendIt = friends.find(update.itemId);
					if (friendIt != friends.end())
					{
						auto fr = friendIt->second;
						if (fr)
						{
							fr->status = update.data.status;
							friendListChanged(FriendListUpdatedEvent{ FriendListUpdateOperation::Update, fr });
						}
					}
				}

				void onFriendRemove(const FriendListUpdateDto& update)
				{
					auto friendIt = friends.find(update.itemId);
					if (friendIt != friends.end())
					{
						auto fr = friendIt->second;
						if (fr)
						{
							friends.erase(update.itemId);
							friendListChanged(FriendListUpdatedEvent{ FriendListUpdateOperation::Remove, fr });
						}
					}
				}

				std::weak_ptr<Scene> _scene;
				std::shared_ptr<ILogger> _logger;
				std::shared_ptr<RpcService> _rpcService;
				bool _isLoaded = false;
			};

			class Friends_Impl : public ClientAPI<Friends_Impl, FriendsService>, public ::Stormancer::Friends::Friends, public ::Stormancer::Users::IAuthenticationEventHandler
			{
			public:

				Friends_Impl(std::weak_ptr<Users::UsersApi> users, std::vector<std::shared_ptr<IFriendsEventHandler>> friendsEventHandlers)
					: ClientAPI(users, "stormancer.friends")
					, _friendsEventHandlers(friendsEventHandlers)
				{
				}

				FriendsResult friends() override
				{
					FriendsResult result;
					if (isLoaded())
					{
						auto service = getFriendService();
						result.isReady = true;
						auto friends = service.get()->friends;

						for (auto& handler : this->_friendsEventHandlers)
						{
							handler->getFriends(friends);
						}
						for (auto f : friends)
						{
							auto key = f.first;
							auto value = f.second;
							result.friends.emplace(key, *value);
						}
					}

					return result;
				}

				bool isLoaded() override
				{
					auto task = getFriendService();
					return task.is_done() && task.get()->isLoaded();
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

				virtual pplx::task<void> refresh() override
				{
					return getFriendService().then([](std::shared_ptr<FriendsService> s)
					{
						return s->refresh();
					});
				}

				Subscription subscribeFriendListUpdatedEvent(std::function<void(FriendListUpdatedEvent)> callback) override
				{
					return friendListChanged.subscribe(callback);
				}

				Event<FriendListUpdatedEvent> friendListChanged;

			private:

				pplx::task<std::shared_ptr<FriendsService>> getFriendService()
				{
					auto initializer = [](auto that, auto friends, auto scene)
					{
						auto wThat = that->weak_from_this();
						that->_friendListChangedSubscription = friends->friendListChanged.subscribe([wThat](FriendListUpdatedEvent ev)
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

					return this->getService(initializer, cleanup);
				}

				Event<FriendListUpdatedEvent>::Subscription _friendListChangedSubscription;
				std::vector<std::shared_ptr<IFriendsEventHandler>> _friendsEventHandlers;
			};
		}

		class FriendsPlugin : public IPlugin
		{
		public:

			void registerSceneDependencies(ContainerBuilder& builder, std::shared_ptr<Scene> scene) override
			{
				auto name = scene->getHostMetadata("stormancer.friends");
				if (name.length() > 0)
				{
					builder.registerDependency<details::FriendsService, Scene, ILogger>().singleInstance();
				}
			}

			void sceneCreated(std::shared_ptr<Scene> scene) override
			{
				auto name = scene->getHostMetadata("stormancer.friends");
				if (!name.empty())
				{
					scene->dependencyResolver().resolve<details::FriendsService>()->initialize();
				}
			}

			void registerClientDependencies(ContainerBuilder& builder) override
			{
				builder.registerDependency<details::Friends_Impl, Users::UsersApi, ContainerBuilder::All<IFriendsEventHandler>>().as<Friends>().singleInstance();
			}
		};
	}
}

MSGPACK_ADD_ENUM(Stormancer::Friends::FriendListUpdateOperationInternal);
MSGPACK_ADD_ENUM(Stormancer::Friends::FriendStatus);
MSGPACK_ADD_ENUM(Stormancer::Friends::FriendListStatusConfig);

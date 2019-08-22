#pragma once
#include "stormancer/Event.h"
#include "stormancer/Tasks.h"
#include "stormancer/msgpack_define.h"
#include "stormancer/StormancerTypes.h"
#include "stormancer/Scene.h"
#include "stormancer/ClientAPI.h"
#include "Users/Users.hpp"
#include "GameFinder/GameFinder.h"
#include "Authentication/AuthenticationService.h" // TODO replace with Users
#include <string>
#include <unordered_map>

namespace Stormancer
{
	namespace Party
	{
		struct PartyUserDto;
		struct PartyUserData;
		struct PartySettings;
		struct PartySettingsUpdate;
		struct PartyInvitation;
		struct PartyRequestDto;

		enum class PartyUserStatus
		{
			NotReady = 0,
			Ready = 1
		};

		struct PartyError
		{
			enum Value
			{
				UnspecifiedError,
				InvalidInvitation,
				AlreadyInParty,
				NotInParty,
				PartyNotReady,
				SettingsUpdated
			};

			struct Str
			{
				static constexpr const char* InvalidInvitation = "party.invalidInvitation";
				static constexpr const char* AlreadyInParty = "party.alreadyInParty";
				static constexpr const char* NotInParty = "party.notInParty";
				static constexpr const char* PartyNotReady = "party.partyNotReady";
				static constexpr const char* SettingsUpdated = "party.settingsUpdated";

				Str() = delete;
			};

			static Value fromString(const char* error)
			{
				if (std::strcmp(error, Str::AlreadyInParty) == 0)		{ return AlreadyInParty; }
					
				if (std::strcmp(error, Str::InvalidInvitation) == 0)	{ return InvalidInvitation; }

				if (std::strcmp(error, Str::NotInParty) == 0)			{ return NotInParty; }

				if (std::strcmp(error, Str::PartyNotReady) == 0)		{ return PartyNotReady; }

				return UnspecifiedError;
			}

			PartyError() = delete;
		};


		class PartyApi
		{
		public:
			/// <summary>
			/// Virtual destructor
			/// </summary>
			virtual ~PartyApi() {}

			/// <summary>
			/// Create a party and set the player as leader
			/// </summary>
			/// <remarks>
			/// If the player is currently in a party, the operation fails.
			/// </remarks>
			/// <param name="partyRequest">Party creation parameters</param>
			/// <returns></returns>
			virtual pplx::task<void> createParty(const PartyRequestDto& partyRequest) = 0;

			/// <summary>
			/// Join an existing party using a connection token provided by the server
			/// </summary>
			/// <param name="connectionToken">Token required to connect to the party.</param>
			/// <returns>A task that completes once the party has been joined.</returns>
			virtual pplx::task<void> joinParty(const std::string& connectionToken) = 0;

			/// <summary>
			/// Join an existing party that you were invited to.
			/// </summary>
			/// <param name="invitation">The invitation that you want to accept.</param>
			/// <returns>A task that completes once the party has been joined.</returns>
			virtual pplx::task<void> joinParty(const PartyInvitation& invitation) = 0;

			/// <summary>
			/// Leave the party
			/// </summary>
			/// <returns>A task that completes with the operation.</returns>
			virtual pplx::task<void> leaveParty() = 0;

			/// <summary>
			/// Check if you are currently in a party.
			/// </summary>
			/// <returns>
			/// <c>true</c> if you are in a party, <c>false</c> otherwise.
			/// Note that if you are in the process of joining or creating a party, but are not finished yet, this method will also return <c>false</c>.
			/// </returns>
			virtual bool isInParty() const = 0;

			/// <summary>
			/// Get the members of the currently joined party.
			/// </summary>
			/// <returns>A vector of structs that describe every user who is currently in the party.</returns>
			virtual std::vector<PartyUserDto> getPartyMembers() const = 0;

			/// <summary>
			/// Set the player status.
			/// When all players in party are ready, gamefinding is automatically started.
			/// </summary>
			/// <param name="playerStatus">Ready or not ready</param>
			/// <returns>A task that completes when the update has been sent.</returns>
			virtual pplx::task<void> updatePlayerStatus(PartyUserStatus playerStatus) = 0;

			/// <summary>
			/// Get the settings of the current party.
			/// </summary>
			/// <returns>The settings of the current party, if the current user is currently in a party.</returns>
			virtual PartySettings getPartySettings() const = 0;

			/// <summary>
			/// Update the party settings
			/// </summary>
			/// <remarks>
			/// Party settings can only be set by the party leader.
			/// </remarks>
			/// <remarks>
			/// Party settings are automatically replicated to other players. The current value is available
			/// in the current party object. Subscribe to the onUpdatedPartySettings event to listen to update events.
			/// </remarks>
			/// <param name="partySettings"></param>
			/// <returns></returns>
			virtual pplx::task<void> updatePartySettings(PartySettingsUpdate partySettings) = 0;

			/// <summary>
			/// Update the data associated with the player
			/// </summary>
			/// <remarks>
			/// player data are automatically replicated to other players. The current value is available
			/// in the current party members list. Subscribe to the onUpdatedUserData event to listen to update events.
			/// </remarks>
			/// <param name="data"></param>
			/// <returns></returns>
			virtual pplx::task<void> updatePlayerData(std::string data) = 0;

			/// <summary>
			/// Check if the local user is the leader of the party.
			/// </summary>
			/// <returns><c>true</c> if the local user is the leader, <c>false</c> otherwise.</returns>
			virtual bool isLeader() const = 0;

			/// <summary>
			/// Promote the specified user as leader
			/// </summary>
			/// <remarks>
			/// The caller must be the leader of the party
			/// </remarks>
			/// <remarks>
			/// The new leader must be in the party
			/// </remarks>
			/// <param name="userId">The id of the player to promote</param>
			/// <returns></returns>
			virtual pplx::task<bool> promoteLeader(std::string userId) = 0;

			/// <summary>
			/// Kick the specified user from the party
			/// </summary>
			/// <remarks>
			/// The caller must be the leader of the party
			/// </remarks>
			/// <remarks>
			/// If the user has already left the party, the operation succeeds.
			/// </remarks>
			/// <param name="userId">The id of the player to kick</param>
			/// <returns></returns>
			virtual pplx::task<bool> kickPlayer(std::string userId) = 0;

			/// <summary>
			/// Invite a player to join the party.
			/// </summary>
			/// <param name="userId">The stormancer id of the player to invite.</param>
			/// <param name="ct">A token that can be used to cancel the invitation.</param>
			/// <returns>
			/// A task that completes when the recipient has either:
			/// - accepted the invitation
			/// - declined the invitation
			/// - quit the game.
			/// </returns>
			virtual pplx::task<void> invitePlayer(const std::string& userId, pplx::cancellation_token ct = pplx::cancellation_token::none()) = 0;

			/// <summary>
			/// Get pending party invitations for the player.
			/// </summary>
			/// <returns>A vector of invitations that have been received and have not yet been accepted.</returns>
			virtual std::vector<PartyInvitation> getPendingInvitations() = 0;

			virtual Event<PartySettings>::Subscription subscribeOnUpdatedPartySettings(std::function<void(PartySettings)> callback) = 0;
			virtual Event<std::vector<PartyUserDto>>::Subscription subscribeOnUpdatedPartyMembers(std::function<void(std::vector<PartyUserDto>)> callback) = 0;
			virtual Event<PartyUserData>::Subscription subscribeOnUpdatedUserData(std::function<void(PartyUserData)> callback) = 0;
			virtual Event<void>::Subscription subscribeOnJoinedParty(std::function<void()> callback) = 0;
			virtual Event<void>::Subscription subscribeOnKickedFromParty(std::function<void()> callback) = 0;
			virtual Event<void>::Subscription subscribeOnLeftParty(std::function<void()> callback) = 0;
			virtual Event<PartyInvitation>::Subscription subscribeOnInvitationReceived(std::function<void(PartyInvitation)> callback) = 0;
			virtual Event<std::string>::Subscription subscribeOnInvitationCanceled(std::function<void(std::string)> callback) = 0;
		};

		struct PartyRequestDto
		{
			std::string platformSessionId;
			std::string GameFinderName;
			std::string CustomData;
			MSGPACK_DEFINE(platformSessionId, GameFinderName, CustomData);
		};

		struct PartyInvitation
		{
			std::string UserId;
			std::string SceneId;
			PartyInvitation(std::string userId, std::string sceneId)
			{
				UserId = userId;
				SceneId = sceneId;
			}
		};

		struct PartyUserDto
		{
			std::string userId;
			PartyUserStatus partyUserStatus;
			std::string userData;

			bool isLeader = false; // Computed locally

			MSGPACK_DEFINE(userId, partyUserStatus, userData)
		};

		struct PartyUserData
		{
			std::string userId;
			std::string userData;
			MSGPACK_DEFINE(userId, userData)
		};

		struct PartySettings
		{
			/// <summary>
			/// The name of game finder scene
			/// </summary>
			std::string gameFinderName;

			/// <summary>
			/// The leader user id
			/// </summary>
			std::string leaderId;

			std::string customData;

			MSGPACK_DEFINE(gameFinderName, leaderId, customData)
		};

		struct PartySettingsUpdate
		{
			std::string gameFinderName;
			std::string customData;

			MSGPACK_DEFINE(gameFinderName, customData)
		};


		namespace details
		{
			class PartyService : public std::enable_shared_from_this<PartyService>
			{
			public:

				PartyService(std::weak_ptr<Scene> scene)
					: _scene(scene)
					, _logger(scene.lock()->dependencyResolver().resolve<ILogger>())
					, _rpcService(_scene.lock()->dependencyResolver().resolve<RpcService>())
					, _gameFinder(scene.lock()->dependencyResolver().resolve<GameFinder>())
					, _myUserId(scene.lock()->dependencyResolver().resolve<AuthenticationService>()->userId())
				{}

				~PartyService()
				{
					std::lock_guard<std::recursive_mutex> lg(_stateMutex);

					_gameFinderConnectionTask.then([](pplx::task<void> task)
					{
						try { task.get(); } catch (...) {}
					});
				}

				///
				/// Sent to server the new party status
				///
				pplx::task<void> updatePartySettings(const PartySettingsUpdate& newPartySettings)
				{
					return _rpcService->rpc<void>("party.updatepartysettings", newPartySettings);
				}

				/// 
				/// Set our party status (ready/not ready).
				/// Also make sure that we are connected to the party's GameFinder before telling the server that we're ready.
				/// 
				pplx::task<void> updatePlayerStatus(const PartyUserStatus newStatus)
				{
					std::lock_guard<std::recursive_mutex> lg(_stateMutex);

					bool statusHasChanged = std::any_of(_members.begin(), _members.end(),
						[newStatus, this](const auto& member) { return member.userId == _myUserId && member.partyUserStatus != newStatus; });

					if (!statusHasChanged)
					{
						return pplx::task_from_result();
					}
					if (_settings.gameFinderName.empty())
					{
						return pplx::task_from_exception<void>(std::runtime_error(PartyError::Str::PartyNotReady));
					}

					if (newStatus == PartyUserStatus::NotReady)
					{
						return _rpcService->rpc<void>("party.updategamefinderplayerstatus", newStatus);
					}

					// When setting our status to Ready, we need to account for the case when the settings change during the ready RPC
					std::weak_ptr<PartyService> wThat = this->shared_from_this();
					return _gameFinderConnectionTask.then([newStatus, wThat](pplx::task<void> task)
					{
						auto that = wThat.lock();
						try
						{
							auto status = task.wait();
							if (status == pplx::canceled)
							{
								return pplx::task_from_exception<void>(std::runtime_error(PartyError::Str::SettingsUpdated));
							}
							if (that)
							{
								return that->_rpcService->rpc<void>("party.updategamefinderplayerstatus", newStatus);
							}
							else
							{
								pplx::cancel_current_task();
							}
						}
						catch (...)
						{
							// Error connecting to the game finder ; we're about to leave the party
							pplx::cancel_current_task();
						}
					});
				}

				/// 
				/// Update party user data all data are replecated between all connected party scene
				/// 
				pplx::task<void> updatePlayerData(std::string data)
				{
					return _rpcService->rpc<void>("party.updatepartyuserdata", data);
				}

				///
				/// Promote player to leader of the party
				/// \param playerId party userid will be promote
				pplx::task<bool> promoteLeader(const std::string playerId)
				{
					return _rpcService->rpc<bool>("party.promoteleader", playerId);
				}

				///
				/// Remove player from party this method can be call only by party leader.
				/// \param playerToKick is the user player id to be kicked
				pplx::task<bool> kickPlayer(const std::string playerId)
				{
					return _rpcService->rpc<bool>("party.kickplayer", playerId);
				}

				///
				/// Callback member
				///
				Event<GameFinderStatus> PartyGameFinderStateUpdated;
				Event<GameFinderResponse> onPartyGameFound;
				Event<void> LeftParty;
				Event<void> JoinedParty;
				Event<void> KickedFromParty;
				Event<std::vector<PartyUserDto>> UpdatedPartyMembers;
				Event<PartyUserData> UpdatedPartyUserData;
				Event<PartySettings> UpdatedPartySettings;

				std::vector<PartyUserDto> members() const
				{
					std::lock_guard<std::recursive_mutex> lg(_stateMutex);
					return _members;
				}

				PartySettings settings() const
				{
					std::lock_guard<std::recursive_mutex> lg(_stateMutex);
					return _settings;
				}

				void initialize()
				{
					std::weak_ptr<PartyService> wThat = this->shared_from_this();

					_scene.lock()->addRoute("party.updatesettings", [wThat](Packetisp_ptr data)
					{
						auto that = wThat.lock();
						if (that)
						{
							auto updatedSettings = data->readObject<PartySettings>();
							that->setNewLocalSettings(updatedSettings);
						}
					});

					_scene.lock()->addRoute("party.updateuserdata", [wThat](Packetisp_ptr data)
					{
						if (auto that = wThat.lock())
						{
							auto updatedPartyUserData = data->readObject<PartyUserData>();
							that->updateUserData(updatedPartyUserData);
						}
					});

					_scene.lock()->addRoute("party.updatepartymembers", [wThat](Packetisp_ptr data)
					{
						if (auto that = wThat.lock())
						{
							auto members = data->readObject<std::vector<PartyUserDto>>();
							that->updatePartyMembers(members);
						}
					});

					_scene.lock()->addRoute("party.kicked", [wThat](Packetisp_ptr data)
					{
						if (auto that = wThat.lock())
						{
							that->KickedFromParty();
						}
					});

					_scene.lock()->getConnectionStateChangedObservable().subscribe([wThat](ConnectionState state) {
						if (auto that = wThat.lock())
						{
							if (state == ConnectionState::Connected)
							{
								that->JoinedParty();
							}
							else if (state == ConnectionState::Disconnected)
							{
								that->_gameFinder->disconnectFromGameFinder(that->_settings.gameFinderName)
									.then([](pplx::task<void> t)
								{
									try {
										t.get();
									}
									catch (...) {}
								});

								that->LeftParty();
							}
						}

					});
				}

				pplx::task<void> waitForPartyReady()
				{
					auto tasks = { pplx::create_task(_memberUpdateReceived), pplx::create_task(_settingsUpdateReceived) };
					return pplx::when_all(tasks.begin(), tasks.end());
				}

			private:

				void updateGameFinder(std::string newGameFinderName)
				{
					std::lock_guard<std::recursive_mutex> lg(_stateMutex);

					if (newGameFinderName.empty() || newGameFinderName == _settings.gameFinderName)
					{
						return;
					}

					std::string oldGameFinderName = _settings.gameFinderName;
					_settings.gameFinderName = newGameFinderName;

					_logger->log(LogLevel::Trace, "PartyService", "Connecting to the party's new GameFinder", newGameFinderName);

					// This CTS prevents multiple game finder connection requests from queuing up.
					_gameFinderConnectionCts.cancel();
					_gameFinderConnectionCts = pplx::cancellation_token_source();

					// No need to wait for the old GF disconnection before connecting to the new GF
					_gameFinder->disconnectFromGameFinder(_settings.gameFinderName).then([](pplx::task<void> task)
					{
						try { task.get(); }
						catch (...) {}
					});

					std::weak_ptr<PartyService> wThat = this->shared_from_this();
					_gameFinderConnectionTask = _gameFinderConnectionTask.then([wThat, newGameFinderName](pplx::task<void> task)
					{
						// I want to recover from cancellation, but not from error, since error means we're leaving the party
						task.wait();

						auto that = wThat.lock();
						if (!that || pplx::is_task_cancellation_requested())
						{
							pplx::cancel_current_task();
						}

						return that->_gameFinder->connectToGameFinder(newGameFinderName);
					}, _gameFinderConnectionCts.get_token())
						.then([wThat, newGameFinderName]
					{
						auto that = wThat.lock();
						if (!that || pplx::is_task_cancellation_requested())
						{
							pplx::cancel_current_task();
						}

						that->_logger->log(LogLevel::Trace, "PartyService", "Connected to the GameFinder", newGameFinderName);
					})
						.then([wThat, newGameFinderName](pplx::task<void> task)
					{
						auto that = wThat.lock();
						try
						{
							auto status = task.wait();
							if (status == pplx::canceled)
							{
								if (that)
								{
									that->_logger->log(LogLevel::Trace, "PartyService", "Connection to the GameFinder was canceled", newGameFinderName);
								}
								pplx::cancel_current_task();
							}
						}
						catch (const std::exception& ex)
						{
							if (that)
							{
								if (auto scene = that->_scene.lock())
								{
									that->_logger->log(LogLevel::Error, "PartyService", "Error connecting to the GameFinder '"+ newGameFinderName +"' ; now leaving the party", ex);
									scene->disconnect().then([](pplx::task<void> t) { try { t.get(); } catch (...) {} });

									std::lock_guard<std::recursive_mutex> lg(that->_stateMutex);
									that->_scene.reset();
								}
							}
							throw;
						}
					});
				}

				void setNewLocalSettings(const PartySettings& partySettingsDto)
				{
					std::lock_guard<std::recursive_mutex> lg(_stateMutex);

					bool leaderChanged = _settings.leaderId != partySettingsDto.leaderId;
					
					_settings.leaderId = partySettingsDto.leaderId;
					_settings.customData = partySettingsDto.customData;
					updateGameFinder(partySettingsDto.gameFinderName);

					if (leaderChanged && _members.size() > 0)
					{
						updatePartyMembers(_members);
					}

					this->UpdatedPartySettings(_settings);
					_settingsUpdateReceived.set();
				}

				void updatePartyMembers(std::vector<PartyUserDto> members)
				{
					std::lock_guard<std::recursive_mutex> lg(_stateMutex);

					for (auto& member : members)
					{
						if (member.userId == _settings.leaderId)
						{
							member.isLeader = true;
						}
					}
					_members = members;

					// Wait until the first settings update has been received to trigger the event
					if (!_settings.leaderId.empty())
					{
						this->UpdatedPartyMembers(_members);
						_memberUpdateReceived.set();
					}
				}

				void updateUserData(const PartyUserData& data)
				{
					std::lock_guard<std::recursive_mutex> lg(_stateMutex);

					auto member = std::find_if(_members.begin(), _members.end(), [&data](const PartyUserDto& user) { return data.userId == user.userId; });

					if (member != _members.end())
					{
						member->userData = data.userData;
					}

					this->UpdatedPartyUserData(data);
				}

				PartySettings _settings;
				std::weak_ptr<Scene> _scene;
				std::shared_ptr<ILogger> _logger;
				std::shared_ptr<RpcService> _rpcService;
				std::shared_ptr<GameFinder> _gameFinder;

				std::string _myUserId;
				std::vector<PartyUserDto> _members;
				// Synchronize async state update, as well as getters.
				// This is "coarse grain" synchronization, but the simplicity gains vs. multiple mutexes win against the possible performance loss imo.
				mutable std::recursive_mutex _stateMutex;
				// Prevent having multiple game finder connection tasks at the same time (could happen if multiple settings updates are received in a short amount of time)
				pplx::task<void> _gameFinderConnectionTask = pplx::task_from_result();
				pplx::cancellation_token_source _gameFinderConnectionCts;
				// Used to signal to client code when party settings/members are initially ready
				pplx::task_completion_event<void> _memberUpdateReceived;
				pplx::task_completion_event<void> _settingsUpdateReceived;
			};

			class PartyContainer
			{
			public:
				PartyContainer(
					std::shared_ptr<Scene> scene,
					Event<void>::Subscription LeftPartySubscription,
					Event<void>::Subscription KickedFromPartySubscription,
					Event<std::vector<PartyUserDto>>::Subscription UpdatedPartyMembersSubscription,
					Event<PartyUserData>::Subscription UpdatedPartyUserDataSubscription,
					Event<PartySettings>::Subscription UpdatedPartySettingsSubscription
				)
					: _partyScene(scene)
					, LeftPartySubscription(LeftPartySubscription)
					, KickedFromPartySubscription(KickedFromPartySubscription)
					, UpdatedPartyMembersSubscription(UpdatedPartyMembersSubscription)
					, UpdatedPartyUserDataSubscription(UpdatedPartyUserDataSubscription)
					, UpdatedPartySettingsSubscription(UpdatedPartySettingsSubscription)
				{
				}

				PartySettings settings() const
				{
					return  _partyScene->dependencyResolver().resolve<details::PartyService>()->settings();
				}

				std::vector<PartyUserDto> members() const
				{
					return _partyScene->dependencyResolver().resolve<details::PartyService>()->members();
				}

				bool isLeader() const
				{
					return (settings().leaderId == _partyScene->dependencyResolver().resolve<AuthenticationService>()->userId());
				}

				std::shared_ptr<Scene> getScene() const { return _partyScene; }
				std::string id() const { return _partyScene->id(); }

				void registerInvitationRequest(std::string recipientId, pplx::cancellation_token_source cts)
				{
					std::lock_guard<std::mutex> lg(_invitationsMutex);

					if (_pendingInvitationRequests.find(recipientId) == _pendingInvitationRequests.end())
					{
						_pendingInvitationRequests[recipientId] = cts;
					}
					else
					{
						throw std::runtime_error(&("Could not send a party invitation as there is already one for " + recipientId)[0]);
					}
				}

				void closeInvitationRequest(std::string recipientId)
				{
					std::lock_guard<std::mutex> lg(_invitationsMutex);

					if (_pendingInvitationRequests.find(recipientId) != _pendingInvitationRequests.end())
					{
						_pendingInvitationRequests[recipientId].cancel();
						_pendingInvitationRequests.erase(recipientId);
					}
					else
					{
						throw std::runtime_error(&("Could not cancel a party invitation as there none pending for " + recipientId)[0]);
					}
				}

				~PartyContainer()
				{
					std::lock_guard<std::mutex> lg(_invitationsMutex);

					for (auto& request : _pendingInvitationRequests)
					{
						request.second.cancel();
					}
				}

			private:
				std::shared_ptr<Scene> _partyScene;

				Event<void>::Subscription LeftPartySubscription;
				Event<void>::Subscription KickedFromPartySubscription;
				Event<std::vector<PartyUserDto>>::Subscription UpdatedPartyMembersSubscription;
				Event<PartyUserData>::Subscription UpdatedPartyUserDataSubscription;
				Event<PartySettings>::Subscription UpdatedPartySettingsSubscription;

				std::unordered_map<std::string, pplx::cancellation_token_source> _pendingInvitationRequests;
				std::mutex _invitationsMutex;
			};

			class PartyManagementService : public std::enable_shared_from_this<PartyManagementService>
			{

			public:

				PartyManagementService(std::shared_ptr<Scene> scene)
					: _logger(scene->dependencyResolver().resolve<ILogger>())
					, _scene(scene)
				{}

				pplx::task<std::string> createParty(const  PartyRequestDto& partyRequestDto)
				{
					auto rpc = _scene.lock()->dependencyResolver().resolve<RpcService>();
					return rpc->rpc<std::string, PartyRequestDto>("partymanagement.createsession", partyRequestDto);
				}

			private:
				std::shared_ptr<ILogger> _logger;
				std::weak_ptr<Scene> _scene;

				//Internal callbacks
				std::function<void(RpcRequestContext_ptr requestContext)> _onCreateSession;
			};

			class Party_Impl : public ClientAPI<Party_Impl>, public PartyApi
			{
			public:

				Party_Impl(
					std::weak_ptr<AuthenticationService> auth,
					std::weak_ptr<ILogger> logger,
					std::shared_ptr<IActionDispatcher> dispatcher
				)
					: ClientAPI(auth)
					, _logger(logger)
					, _dispatcher(dispatcher)
				{}

				pplx::task<void> createParty(const PartyRequestDto& partySettings) override
				{
					if (_party)
					{
						return pplx::task_from_exception<void>(std::runtime_error(PartyError::Str::AlreadyInParty));
					}
					auto auth = _auth.lock();
					if (!auth)
					{
						return pplx::task_from_exception<void>(std::runtime_error("destroyed"));
					}

					auto wThat = this->weak_from_this();
					return getPartyManagementService().then([partySettings](std::shared_ptr<PartyManagementService> partyManagement)
					{
						return partyManagement->createParty(partySettings);
					}).then([wThat, partySettings](pplx::task<std::string> task)
					{
						auto sceneToken = task.get();
						auto that = wThat.lock();
						if (!that)
						{
							throw std::runtime_error("destroyed");
						}

						return that->joinParty(sceneToken);
					});
				}

				pplx::task<void> joinParty(const std::string& token) override
				{
					if (_party)
					{
						return pplx::task_from_exception<void>(std::runtime_error(PartyError::Str::AlreadyInParty));
					}

					auto wPartyManagement = this->weak_from_this();
					auto partyTask = leaveParty().then([wPartyManagement, token]
					{
						auto partyManagment = wPartyManagement.lock();
						if (partyManagment)
						{
							return partyManagment->getPartySceneByToken(token);
						}
						throw std::runtime_error("destroyed");
					}, _dispatcher);

					auto userTask = partyTask.then([wPartyManagement](pplx::task<std::shared_ptr<PartyContainer>> t2)
					{
						try
						{
							auto p = t2.get();
							if (auto that = wPartyManagement.lock())
							{
								// Wait for the party task to be complete before triggering these events, to stay consistent with isInParty()
								that->_onJoinedParty();
								that->_onUpdatedPartyMembers(that->getPartyMembers());
								that->_onUpdatedPartySettings(that->getPartySettings());
							}
						}
						catch (std::exception& ex)
						{
							if (auto that = wPartyManagement.lock())
							{
								that->_logger->log(LogLevel::Error, "PartyManagement", "Failed to get the party scene.", ex.what());
								that->_party = nullptr;
							}
							throw;
						}
					}, _dispatcher);

					this->_party = std::make_shared<pplx::task<std::shared_ptr<PartyContainer>>>(partyTask);
					return userTask;
				}

				pplx::task<void> joinParty(const PartyInvitation& invitation) override
				{
					{
						std::lock_guard<std::recursive_mutex> lg(_invitationsMutex);

						auto it = _invitations.find(invitation.UserId);
						if (it == _invitations.end())
						{
							return pplx::task_from_exception<void>(std::runtime_error(PartyError::Str::InvalidInvitation));
						}
						it->second.tce.set();
						_invitations.erase(it);
					}

					auto wThat = this->weak_from_this();
					return _auth.lock()->getSceneConnectionToken("stormancer.plugins.party", invitation.SceneId, pplx::cancellation_token::none())
						.then([wThat](std::string token)
					{
						if (auto that = wThat.lock())
						{
							return that->joinParty(token);
						}
						throw std::runtime_error("destroyed");
					});
				}

				pplx::task<void> leaveParty() override
				{
					if (!_party)
					{
						return pplx::task_from_result();
					}

					auto party = *_party;
					std::weak_ptr<Party_Impl> wpartyManagement = this->weak_from_this();
					return party.then([wpartyManagement](std::shared_ptr<PartyContainer> party)
					{
						if (auto partyManagement = wpartyManagement.lock())
						{
							partyManagement->_party = nullptr;
						}

						return party->getScene()->disconnect();
					});
				}

				bool isInParty() const override
				{
					return _party != nullptr && _party->is_done();
				}

				std::vector<PartyUserDto> getPartyMembers() const override
				{
					if (!isInParty())
					{
						throw std::runtime_error(PartyError::Str::NotInParty);
					}

					return _party->get()->members();
				}

				PartySettings getPartySettings() const override
				{
					if (!isInParty())
					{
						throw std::runtime_error(PartyError::Str::NotInParty);
					}

					return _party->get()->settings();
				}

				bool isLeader() const override
				{
					if (!isInParty())
					{
						throw std::runtime_error(PartyError::Str::NotInParty);
					}

					return _party->get()->isLeader();
				}
				
				// Not const because of mutex lock
				std::vector<PartyInvitation> getPendingInvitations() override
				{
					std::vector<PartyInvitation> pendingInvitations;
					{
						std::lock_guard<std::recursive_mutex> lg(_invitationsMutex);

						for (const auto& it : _invitations)
						{
							pendingInvitations.push_back(it.second.invite);
						}
					}
					return pendingInvitations;
				}

				pplx::task<void> updatePlayerStatus(PartyUserStatus playerStatus) override
				{
					if (!isInParty())
					{
						return pplx::task_from_exception<void>(std::runtime_error(PartyError::Str::NotInParty));
					}

					return _party->then([playerStatus](std::shared_ptr<PartyContainer> party)
					{
						auto partyService = party->getScene()->dependencyResolver().resolve<PartyService>();
						partyService->updatePlayerStatus(playerStatus);
					});
				}

				pplx::task<void> updatePartySettings(PartySettingsUpdate partySettingsDto) override
				{
					if (!isInParty())
					{
						return pplx::task_from_exception<void>(std::runtime_error(PartyError::Str::NotInParty));
					}

					if (partySettingsDto.customData == "")
					{
						partySettingsDto.customData = "{}";
					}

					return _party->then([partySettingsDto](std::shared_ptr<PartyContainer> party) {
						auto partyService = party->getScene()->dependencyResolver().resolve<PartyService>();
						return partyService->updatePartySettings(partySettingsDto);
					});
				}

				pplx::task<void> updatePlayerData(std::string data) override
				{
					if (!isInParty())
					{
						return pplx::task_from_exception<void>(std::runtime_error(PartyError::Str::NotInParty));
					}

					return _party->then([data](std::shared_ptr<PartyContainer> party)
					{
						auto partyService = party->getScene()->dependencyResolver().resolve<PartyService>();
						return partyService->updatePlayerData(data);
					});
				}

				pplx::task<bool> promoteLeader(std::string userId) override
				{
					if (!isInParty())
					{
						return pplx::task_from_exception<bool>(std::runtime_error(PartyError::Str::NotInParty));
					}

					return _party->then([userId](std::shared_ptr<PartyContainer> party) {
						std::shared_ptr<PartyService> partyService = party->getScene()->dependencyResolver().resolve<PartyService>();
						return partyService->promoteLeader(userId);
					});
				}

				pplx::task<bool> kickPlayer(std::string userId) override
				{
					if (!isInParty())
					{
						return pplx::task_from_exception<bool>(std::runtime_error(PartyError::Str::NotInParty));
					}

					return _party->then([userId](std::shared_ptr<PartyContainer> party) {
						std::shared_ptr<PartyService> partyService = party->getScene()->dependencyResolver().resolve<PartyService>();
						return partyService->kickPlayer(userId);
					});
				}

				pplx::task<void> invitePlayer(const std::string& recipient, pplx::cancellation_token ct) override
				{
					if (!isInParty())
					{
						return pplx::task_from_exception<void>(std::runtime_error(PartyError::Str::NotInParty));
					}

					auto wAuth = _auth;
					auto wThat = this->weak_from_this();
					return _party->then([wAuth, wThat, recipient, ct](std::shared_ptr<PartyContainer> party)
					{
						auto auth = wAuth.lock();
						auto that = wThat.lock();
						if (!auth || !that)
						{
							return pplx::task_from_result();
						}

						auto partyId = party->id();

						pplx::cancellation_token_source cts;

						party->registerInvitationRequest(recipient, cts);

						std::weak_ptr<PartyContainer> wParty(party);
						auto requestTask = auth->sendRequestToUser<void>(recipient, "party.invite", cts.get_token(), partyId)
							.then([recipient, wParty]
						{
							if (auto party = wParty.lock())
							{
								party->closeInvitationRequest(recipient);
							}
						});

						if (ct.is_cancelable())
						{
							ct.register_callback([recipient, wParty]
							{
								if (auto party = wParty.lock())
								{
									party->closeInvitationRequest(recipient);
								}
							});
						}

						return requestTask;
					});
				}

				Event<PartySettings>::Subscription subscribeOnUpdatedPartySettings(std::function<void(PartySettings)> callback) override
				{
					return _onUpdatedPartySettings.subscribe(callback);
				}

				Event<std::vector<PartyUserDto>>::Subscription subscribeOnUpdatedPartyMembers(std::function<void(std::vector<PartyUserDto>)> callback) override
				{
					return _onUpdatedPartyMembers.subscribe(callback);
				}

				Event<PartyUserData>::Subscription subscribeOnUpdatedUserData(std::function<void(PartyUserData)> callback) override
				{
					return _onUpdatedUserData.subscribe(callback);
				}

				Event<void>::Subscription subscribeOnJoinedParty(std::function<void()> callback) override
				{
					return _onJoinedParty.subscribe(callback);
				}

				Event<void>::Subscription subscribeOnKickedFromParty(std::function<void()> callback) override
				{
					return _onKickedFromParty.subscribe(callback);
				}

				Event<void>::Subscription subscribeOnLeftParty(std::function<void()> callback) override
				{
					return _onLeftParty.subscribe(callback);
				}

				Event<PartyInvitation>::Subscription subscribeOnInvitationReceived(std::function<void(PartyInvitation)> callback) override
				{
					return _onInvitationReceived.subscribe(callback);
				}

				Event<std::string>::Subscription subscribeOnInvitationCanceled(std::function<void(std::string)> callback) override
				{
					return _onInvitationCanceled.subscribe(callback);
				}

				void initialize()
				{
					auto wThat = this->weak_from_this();
					_auth.lock()->SetOperationHandler("party.invite", [wThat](OperationCtx& ctx)
					{
						if (auto that = wThat.lock())
						{
							return that->invitationHandler(ctx);
						}
						return pplx::task_from_result();
					});
				}

			private:

				struct InvitePair
				{
					PartyInvitation invite;
					pplx::task_completion_event<void> tce;

					InvitePair(PartyInvitation invite) : invite(invite) {}
				};

				// Events
				Event<PartySettings> _onUpdatedPartySettings;
				Event<std::vector<PartyUserDto>> _onUpdatedPartyMembers;
				Event<PartyUserData> _onUpdatedUserData;
				Event<void> _onJoinedParty;
				Event<void> _onKickedFromParty;
				Event<void> _onLeftParty;
				Event<PartyInvitation> _onInvitationReceived;
				Event<std::string> _onInvitationCanceled;

				pplx::task<std::shared_ptr<PartyContainer>> getPartySceneByToken(const std::string& token)
				{
					auto auth = _auth.lock();
					auto wPartyManagement = this->weak_from_this();

					return auth->connectToPrivateSceneByToken(token).then([wPartyManagement](std::shared_ptr<Scene> scene)
					{
						auto pManagement = wPartyManagement.lock();
						if (!pManagement)
						{
							throw PointerDeletedException("partyManagement");
						}
						return pManagement->initPartyFromScene(scene);
					});
				}
				pplx::task<std::shared_ptr<PartyManagementService>> getPartyManagementService()
				{
					return this->getService<PartyManagementService>("stormancer.plugins.partyManagement");
				}

				pplx::task<std::shared_ptr<PartyContainer>> initPartyFromScene(std::shared_ptr<Scene> scene)
				{
					std::weak_ptr<Party_Impl> wPartyManagement = this->shared_from_this();
					std::shared_ptr<PartyService> partyService;
					try
					{
						partyService = scene->dependencyResolver().resolve<PartyService>();
					}
					catch (const DependencyResolutionException&)
					{
						throw std::runtime_error(("The scene " + scene->id() + " does not contain a PartyService").c_str());
					}

					auto party = std::make_shared<PartyContainer>(
						scene,
						partyService->KickedFromParty.subscribe([wPartyManagement]() {
							if (auto partyManagement = wPartyManagement.lock())
							{
								partyManagement->_onKickedFromParty();
							}
						}),
						partyService->LeftParty.subscribe([wPartyManagement]() {
							if (auto partyManagement = wPartyManagement.lock())
							{
								partyManagement->_party = nullptr;
								partyManagement->_onLeftParty();
							}
						}),
						partyService->UpdatedPartyMembers.subscribe([wPartyManagement](std::vector<PartyUserDto> partyUsers) {
							if (auto partyManagement = wPartyManagement.lock())
							{
								if (partyManagement->isInParty())
								{
									partyManagement->_onUpdatedPartyMembers(partyUsers);
								}
							}
						}),
						partyService->UpdatedPartyUserData.subscribe([wPartyManagement](PartyUserData partyUserUpdatedData) {
							if (auto partyManagement = wPartyManagement.lock())
							{
								partyManagement->_onUpdatedUserData(partyUserUpdatedData);
							}
						}),
						partyService->UpdatedPartySettings.subscribe([wPartyManagement](PartySettings settings) {
							if (auto partyManagement = wPartyManagement.lock())
							{
								if (partyManagement->isInParty())
								{
									partyManagement->_onUpdatedPartySettings(settings);
								}
							}
						})
					);

					return partyService->waitForPartyReady().then([party] { return party; });
				}

				pplx::task<void> invitationHandler(OperationCtx& ctx)
				{
					Serializer serializer;
					auto senderId = ctx.originId;
					auto sceneId = serializer.deserializeOne<std::string>(ctx.request->inputStream());
					_logger->log(LogLevel::Trace, "Party_Impl::invitationHandler", "Received an invitation: sender="+senderId+" ; sceneId="+sceneId);

					InvitePair invitation(PartyInvitation(senderId, sceneId));
					{
						std::lock_guard<std::recursive_mutex> lg(_invitationsMutex);
						// If we have an older invitation from the same person (it should not be possible, but with the async nature of things...), cancel it first
						auto it = _invitations.find(senderId);
						if (it != _invitations.end())
						{
							_logger->log(LogLevel::Trace, "Party_Impl::invitationHandler", "We already have an invite from this user, cancelling it");
							it->second.tce.set();
							_invitations.erase(it);
							_onInvitationCanceled(senderId);
						}
						_invitations.insert({ senderId, invitation });
					}
					_onInvitationReceived(invitation.invite);

					std::weak_ptr<Party_Impl> wThat(this->shared_from_this());
					auto dispatcher = _dispatcher;
					ctx.request->cancellationToken().register_callback([wThat, senderId, dispatcher]
					{
						pplx::create_task([wThat, senderId]
						{
							if (auto that = wThat.lock())
							{
								that->_logger->log(LogLevel::Trace, "Party_Impl::invitationHandler", "Sender (id="+senderId+") canceled an invitation");
								{
									std::lock_guard<std::recursive_mutex> lg(that->_invitationsMutex);
									that->_invitations.erase(senderId);
								}
								that->_onInvitationCanceled(senderId);
							}
						}, dispatcher);
					});
					return pplx::create_task(invitation.tce);
				}

				std::shared_ptr<ILogger> _logger;
				std::shared_ptr<pplx::task<std::shared_ptr<PartyContainer>>> _party;
				std::unordered_map<std::string, InvitePair> _invitations;
				// Recursive mutex needed because the user can call getPendingInvitations() while in a callback where the mutex is already held
				std::recursive_mutex _invitationsMutex;
				std::shared_ptr<IActionDispatcher> _dispatcher;
			};
		}

		class PartyPlugin : public IPlugin
		{
			void registerSceneDependencies(ContainerBuilder& builder, std::shared_ptr<Scene> scene)
			{
				auto name = scene->getHostMetadata("stormancer.party.v2");
				if (name.length() > 0)
				{
					builder.registerDependency<details::PartyService, Scene>().singleInstance();
				}

				name = scene->getHostMetadata("stormancer.partymanagement.v2");
				if (name.length() > 0)
				{
					builder.registerDependency<details::PartyManagementService, Scene>().singleInstance();
				}
			}

			void sceneCreated(std::shared_ptr<Scene> scene)
			{
				if (!scene->getHostMetadata("stormancer.party.v2").empty())
				{
					scene->dependencyResolver().resolve<details::PartyService>()->initialize();
				}
			}

			void registerClientDependencies(ContainerBuilder& builder)
			{
				builder.registerDependency<PartyApi>([](const DependencyScope& dr) {
					auto partyImpl = std::make_shared<details::Party_Impl>(
						dr.resolve<AuthenticationService>(),
						dr.resolve<ILogger>(),
						dr.resolve<IActionDispatcher>()
					);
					partyImpl->initialize();
					return partyImpl;
				}).singleInstance();
			}
		};
	}
}

MSGPACK_ADD_ENUM(Stormancer::Party::PartyUserStatus)
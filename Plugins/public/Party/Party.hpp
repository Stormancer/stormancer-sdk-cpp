// MIT License
//
// Copyright (c) 2019 Stormancer
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once
#include "stormancer/Event.h"
#include "stormancer/Tasks.h"
#include "stormancer/msgpack_define.h"
#include "stormancer/StormancerTypes.h"
#include "stormancer/Scene.h"
#include "Users/ClientAPI.hpp"
#include "Users/Users.hpp"
#include "GameFinder/GameFinder.hpp"
#include <string>
#include <unordered_map>

namespace Stormancer
{
	namespace Party
	{
		struct PartyUserDto;
		struct PartySettings;
		struct PartyInvitation;
		struct PartyRequestDto;
		struct PartyGameFinderFailure;

		enum class PartyUserStatus
		{
			NotReady = 0,
			Ready = 1
		};

		enum class PartyGameFinderStatus
		{
			SearchStopped = 0,
			SearchInProgress = 1
		};

		enum class MemberDisconnectionReason
		{
			Left = 0,
			Kicked = 1
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
				SettingsOutdated
			};

			struct Str
			{
				static constexpr const char* InvalidInvitation = "party.invalidInvitation";
				static constexpr const char* AlreadyInParty = "party.alreadyInParty";
				static constexpr const char* NotInParty = "party.notInParty";
				static constexpr const char* PartyNotReady = "party.partyNotReady";
				static constexpr const char* SettingsOutdated = "party.settingsOutdated";

				Str() = delete;
			};

			static Value fromString(const char* error)
			{
				if (std::strcmp(error, Str::AlreadyInParty) == 0) { return AlreadyInParty; }

				if (std::strcmp(error, Str::InvalidInvitation) == 0) { return InvalidInvitation; }

				if (std::strcmp(error, Str::NotInParty) == 0) { return NotInParty; }

				if (std::strcmp(error, Str::PartyNotReady) == 0) { return PartyNotReady; }

				if (std::strcmp(error, Str::SettingsOutdated) == 0) { return SettingsOutdated; }

				return UnspecifiedError;
			}

			PartyError() = delete;
		};


		class PartyApi
		{
		public:
			virtual ~PartyApi() = default;

			/// <summary>
			/// Create and join a new party.
			/// </summary>
			/// <remarks>
			/// If the local player is currently in a party, the operation fails.
			/// The local player will be the leader of the newly created party.
			/// </remarks>
			/// <param name="partyRequest">Party creation parameters</param>
			/// <returns>A task that completes when the party has been created and joined.</returns>
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
			/// Get the member list of the currently joined party.
			/// </summary>
			/// <remarks>
			/// It is invalid to call this method while not in a party.
			/// Call <c>isInParty()</c> to check.
			/// </remarks>
			/// <returns>A vector of structs that describe every user who is currently in the party.</returns>
			/// <exception cref="std::exception">If you are not in a party.</exception>
			virtual std::vector<PartyUserDto> getPartyMembers() const = 0;

			/// <summary>
			/// Get the local member's party data.
			/// </summary>
			/// <remarks>
			/// This method is a shortcut for calling <c>getPartyMembers()</c> and iterating over the list to find the local member.
			/// </remarks>
			/// <remarks>
			/// It is invalid to call this method while not in a party.
			/// Call <c>isInParty()</c> to check.
			/// </remarks>
			/// <returns>The struct containing the local player's party data.</returns>
			/// <exception cref="std::exception">If you are not in a party.</exception>
			virtual PartyUserDto getLocalMember() const = 0;

			/// <summary>
			/// Set the local player's status (ready/not ready).
			/// </summary>
			/// <remarks>
			/// By default, a GameFinder request (matchmaking group queuing) is automatically started when all players in the party are ready.
			/// This behavior can be controlled server-side. See the Party documentation for details.
			/// </remarks>
			/// <param name="playerStatus">Ready or not ready</param>
			/// <returns>A task that completes when the update has been sent.</returns>
			/// <exception cref="std::exception">If you are not in a party.</exception>
			virtual pplx::task<void> updatePlayerStatus(PartyUserStatus playerStatus) = 0;

			/// <summary>
			/// Get the settings of the current party.
			/// </summary>
			/// <returns>The settings of the current party, if the current user is currently in a party.</returns>
			/// <exception cref="std::exception">If you are not in a party.</exception>
			virtual PartySettings getPartySettings() const = 0;

			/// <summary>
			/// Get the User Id of the party leader.
			/// </summary>
			/// <returns>The Stormancer User Id of the party leader.</returns>
			/// <exception cref="std::exception">If you are not in a party.</exception>
			virtual std::string getPartyLeaderId() const = 0;

			/// <summary>
			/// Update the party settings
			/// </summary>
			/// <remarks>
			/// Party settings can only be set by the party leader.
			/// Party settings are automatically replicated to other players. The current value is available
			/// in the current party object. Subscribe to the onUpdatedPartySettings event to listen to update events.
			/// </remarks>
			/// <param name="partySettings">New settings</param>
			/// <returns>A task that completes when the settings have been updated and replicated to other players.</returns>
			/// <exception cref="std::exception">If you are not in a party.</exception>
			virtual pplx::task<void> updatePartySettings(PartySettings partySettings) = 0;

			/// <summary>
			/// Update the data associated with the local player
			/// </summary>
			/// <remarks>
			/// player data are automatically replicated to other players. The current value is available
			/// in the current party members list. Subscribe to the OnUpdatedPartyMembers event to listen to update events.
			/// </remarks>
			/// <param name="data">New player data</param>
			/// <returns>A task that completes when the data has been updated and replicated to other players.</returns>
			/// <exception cref="std::exception">If you are not in a party.</exception>
			virtual pplx::task<void> updatePlayerData(std::string data) = 0;

			/// <summary>
			/// Check if the local user is the leader of the party.
			/// </summary>
			/// <returns><c>true</c> if the local user is the leader, <c>false</c> otherwise.</returns>
			/// <exception cref="std::exception">If you are not in a party.</exception>
			virtual bool isLeader() const = 0;

			/// <summary>
			/// Promote the specified user as leader
			/// </summary>
			/// <remarks>
			/// The caller must be the leader of the party
			/// The new leader must be in the party
			/// </remarks>
			/// <param name="userId">The id of the player to promote</param>
			/// <returns>A task that completes when the underlying RPC (remote procedure call) has returned.</returns>
			/// <exception cref="std::exception">If you are not in a party.</exception>
			virtual pplx::task<void> promoteLeader(std::string userId) = 0;

			/// <summary>
			/// Kick the specified user from the party
			/// </summary>
			/// <remarks>
			/// The caller must be the leader of the party
			/// If the user has already left the party, the operation succeeds.
			/// </remarks>
			/// <param name="userId">The id of the player to kick</param>
			/// <returns>A task that completes when the underlying RPC (remote procedure call) has returned.</returns>
			/// <exception cref="std::exception">If you are not in a party.</exception>
			virtual pplx::task<void> kickPlayer(std::string userId) = 0;

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
			/// <exception cref="std::exception">If you are not in a party.</exception>
			virtual pplx::task<void> invitePlayer(const std::string& userId, pplx::cancellation_token ct = pplx::cancellation_token::none()) = 0;

			/// <summary>
			/// Cancels an invitation to join the party
			/// </summary>
			/// <param name="userId">The stormancer id of the player that was invited.</param>
			/// <returns>
			/// A task that completes when the cancellation has been acknowledged.
			/// </returns>
			/// <exception cref="std::exception">If you are not in a party.</exception>
			virtual pplx::task<void> cancelPartyInvitation(std::string recipient) = 0;

			/// <summary>
			/// Get pending party invitations for the player.
			/// </summary>
			/// <remarks>
			/// Call <c>subscribeOnInvitationReceived()</c> in order to be notified when an invitation is received.
			/// </remarks>
			/// <returns>A vector of invitations that have been received and have not yet been accepted.</returns>
			virtual std::vector<PartyInvitation> getPendingInvitations() = 0;

			/// <summary>
			/// Get invitations the player sent in the party.
			/// </summary>
			/// <returns>A vector of user ids to which invitations have been sent but not yet accepted or refused.</returns>
			virtual std::vector<std::string> getSentPendingInvitations() = 0;

			/// <summary>
			/// Register a callback to be run when the party leader changes the party settings.
			/// </summary>
			/// <param name="callback">Callable object taking a <c>PartySettings</c> struct as parameter.</param>
			/// <returns>A <c>Subscription</c> object to track the lifetime of the subscription.</returns>
			virtual Event<PartySettings>::Subscription subscribeOnUpdatedPartySettings(std::function<void(PartySettings)> callback) = 0;
			/// <summary>
			/// Register a callback to be run when the party member list changes.
			/// </summary>
			/// <remarks>
			/// This event is triggered for any kind of change to the list:
			/// - Member addition and removal
			/// - Member data change
			/// - Member status change
			/// - Leader change
			/// The list of <c>PartyUserDto</c> passed to the callback contains only the entries that have changed.
			/// To retrieve the updated full list of members, call <c>getPartyMembers()</c> (it is safe to call from inside the callback too).
			/// </remarks>
			/// <param name="callback">Callable object taking a vector of <c>PartyUserDto</c> structs as parameter.</param>
			/// <returns>A <c>Subscription</c> object to track the lifetime of the subscription.</returns>
			virtual Event<std::vector<PartyUserDto>>::Subscription subscribeOnUpdatedPartyMembers(std::function<void(std::vector<PartyUserDto>)> callback) = 0;
			/// <summary>
			/// Register a callback to be run when the local player has joined a party.
			/// </summary>
			/// <param name="callback">Callable object.</param>
			/// <returns>A <c>Subscription</c> object to track the lifetime of the subscription.</returns>
			virtual Event<void>::Subscription subscribeOnJoinedParty(std::function<void()> callback) = 0;
			/// <summary>
			/// Register a callback to be run when the local player has left the party.
			/// </summary>
			/// <remarks>
			/// The callback parameter <c>MemberDisconnectionReason</c> will be set to <c>Kicked</c> if you were kicked by the party leader.
			/// In any other case, it will be set to <c>Left</c>.
			/// </remarks>
			/// <param name="callback">Callable object taking a <c>MemberDisconnectionReason</c> parameter.</param>
			/// <returns>A <c>Subscription</c> object to track the lifetime of the subscription.</returns>
			virtual Event<MemberDisconnectionReason>::Subscription subscribeOnLeftParty(std::function<void(MemberDisconnectionReason)> callback) = 0;
			/// <summary>
			/// Register a callback to be run when the local player receives an invitation to a party from a remote player.
			/// </summary>
			/// <remarks>
			/// To accept the invitation, call <c>joinParty(PartyInvitation)</c>.
			/// To retrieve the list of every pending invitations received by the local player, call <c>getPendingInvitations()</c>.
			/// </remarks>
			/// <param name="callback">Callable object taking a <c>PartyInvitation</c> parameter.</param>
			/// <returns>A <c>Subscription</c> object to track the lifetime of the subscription.</returns>
			virtual Event<PartyInvitation>::Subscription subscribeOnInvitationReceived(std::function<void(PartyInvitation)> callback) = 0;
			/// <summary>
			/// Register a callback to be run when an invitation sent to the local player was canceled by the sender.
			/// </summary>
			/// <param name="callback">Callable object taking the Id of the user who canceled the invitation.</param>
			/// <returns>A <c>Subscription</c> object to track the lifetime of the subscription.</returns>
			virtual Event<std::string>::Subscription subscribeOnInvitationCanceled(std::function<void(std::string)> callback) = 0;
			
			/// <summary>
			/// Register a callback to be run when the status of the GameFinder for this party is updated.
			/// </summary>
			/// <remarks>
			/// Monitoring the status of the GameFinder can be useful to provide visual feedback to the player.
			/// </remarks>
			/// <param name="callback">Callable object taking a <c>GameFinderStatus</c>.</param>
			/// <returns>A <c>Subscription</c> object to track the lifetime of the subscription.</returns>
			virtual Subscription subscribeOnGameFinderStatusUpdate(std::function<void(PartyGameFinderStatus)> callback) = 0;
			/// <summary>
			/// Register a callback to be run when a game session has been found for this party.
			/// </summary>
			/// <remarks>
			/// This event happens as a result of a successful GameFinder request. Call <c>subscribeOnGameFinderStatusUpdate()</c> to monitor the state of the request.
			/// </remarks>
			/// <param name="callback">Callable object taking a <c>GameFinder::GameFinderResponse</c> containing the information needed to join the game session.</param>
			/// <returns>A <c>Subscription</c> object to track the lifetime of the subscription.</returns>
			virtual Subscription subscribeOnGameFound(std::function<void(GameFinder::GameFinderResponse)> callback) = 0;
			/// <summary>
			/// Register a callback to be run when an error occurs while looking for a game session. 
			/// </summary>
			/// <remarks>
			/// This event is triggered when an ongoing GameFinder request for this party fails for any reason.
			/// GameFinder failure conditions are fully customizable on the server side ; please see the GameFinder documentation for details.
			/// </remarks>
			/// <param name="callback">Callable object taking a <c>PartyGameFinderFailure</c> containing details about the failure.</param>
			/// <returns>A <c>Subscription</c> object to track the lifetime of the subscription.</returns>
			virtual Subscription subscribeOnGameFinderFailure(std::function<void(PartyGameFinderFailure)> callback) = 0;
		};

		struct PartyRequestDto
		{
			std::string platformSessionId;
			std::string GameFinderName;
			std::string CustomData;

			MSGPACK_DEFINE(platformSessionId, GameFinderName, CustomData)
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

			PartyUserDto(std::string userId) : userId(userId) {}
			PartyUserDto() = default;

			MSGPACK_DEFINE(userId, partyUserStatus, userData)
		};

		struct PartySettings
		{
			std::string gameFinderName;
			std::string customData;

			MSGPACK_DEFINE(gameFinderName, customData)
		};

		struct PartyGameFinderFailure
		{
			std::string reason;
		};

		class IPartyEventHandler
		{
		public:
			virtual ~IPartyEventHandler() = default;

			/// <summary>
			/// This event is fired when a Party has been joined, before the completion of createParty()/joinParty() tasks.
			/// </summary>
			/// <remarks>
			/// This gives you a chance to add additional operations as part of the JoinParty process.
			/// For instance, you could join a platform-specific online session, as an alternative to implementing this functionality in the server application.
			/// </remarks>
			/// <param name="party">The general Party API</param>
			/// <param name="partySceneId">Id of the party's scene</param>
			/// <returns>
			/// A task that should complete when your custom operation is done.
			/// If this task is faulted or canceled, the user will be disconnected from the party immediately.
			/// </returns>
			virtual pplx::task<void> onJoiningParty(std::shared_ptr<PartyApi> party, std::string partySceneId) = 0;

			/// <summary>
			/// This event is fired upon leaving the Party you were previously in.
			/// </summary>
			/// <remarks>
			/// This gives you a chance to perform additional operations when you are leaving a party.
			/// For instance, if you joined a platform-specific online session in <c>onJoiningParty()</c>,
			/// you probably want to leave this session in <c>onLeavingParty()</c>.
			/// </remarks>
			/// <param name="party">The general Party API</param>
			/// <param name="partySceneId">Id of the party's scene</param>
			/// <returns>
			/// A task that should complete when your custom operation is done.
			/// </returns>
			virtual pplx::task<void> onLeavingParty(std::shared_ptr<PartyApi> party, std::string partySceneId) = 0;
		};

		namespace details
		{
			struct PartySettingsInternal
			{
				std::string gameFinderName;
				std::string customData;
				int settingsVersionNumber = 0;

				operator PartySettings() const
				{
					PartySettings settings;
					settings.gameFinderName = gameFinderName;
					settings.customData = customData;
					return settings;
				}

				MSGPACK_DEFINE(gameFinderName, customData, settingsVersionNumber)
			};

			struct InvitationRequest
			{
				pplx::cancellation_token_source cts;
				pplx::task<void> task;
			};

			struct PartyState
			{
				PartySettingsInternal		settings;
				std::string					leaderId;
				std::vector<PartyUserDto>	members;
				int							version = 0;

				MSGPACK_DEFINE(settings, leaderId, members, version)
			};

			struct MemberStatusUpdateRequest
			{
				PartyUserStatus	desiredStatus;
				int				localSettingsVersion;

				MSGPACK_DEFINE(desiredStatus, localSettingsVersion)
			};

			struct MemberStatusUpdate
			{
				std::string		userId;
				PartyUserStatus	status;

				MSGPACK_DEFINE(userId, status)
			};

			struct BatchStatusUpdate
			{
				std::vector<MemberStatusUpdate> memberStatus;

				MSGPACK_DEFINE(memberStatus)
			};

			struct PartyUserData
			{
				std::string userId;
				std::string userData;

				MSGPACK_DEFINE(userId, userData)
			};

			struct MemberDisconnection
			{
				std::string					userId;
				MemberDisconnectionReason	reason;

				MSGPACK_DEFINE(userId, reason)
			};

			class PartyService : public std::enable_shared_from_this<PartyService>
			{
			public:

				PartyService(std::weak_ptr<Scene> scene)
					: _scene(scene)
					, _logger(scene.lock()->dependencyResolver().resolve<ILogger>())
					, _rpcService(_scene.lock()->dependencyResolver().resolve<RpcService>())
					, _gameFinder(scene.lock()->dependencyResolver().resolve<Stormancer::GameFinder::GameFinderApi>())
					, _myUserId(scene.lock()->dependencyResolver().resolve<Stormancer::Users::UsersApi>()->userId())
				{}

				~PartyService()
				{
					std::lock_guard<std::recursive_mutex> lg(_stateMutex);

					_gameFinderConnectionTask.then([](pplx::task<void> task)
						{
							try { task.get(); }
							catch (...) {}
						});
				}

				///
				/// Sent to server the new party status
				///
				pplx::task<void> updatePartySettings(const PartySettings& newPartySettings)
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

					bool statusHasChanged = std::any_of(_state.members.begin(), _state.members.end(),
						[newStatus, this](const auto& member) { return member.userId == _myUserId && member.partyUserStatus != newStatus; });

					if (!statusHasChanged)
					{
						return pplx::task_from_result();
					}
					if (_state.settings.gameFinderName.empty())
					{
						return pplx::task_from_exception<void>(std::runtime_error(PartyError::Str::PartyNotReady));
					}

					MemberStatusUpdateRequest request;
					request.desiredStatus = newStatus;
					request.localSettingsVersion = _state.settings.settingsVersionNumber;

					if (request.desiredStatus == PartyUserStatus::NotReady)
					{
						return _rpcService->rpc<void>("party.updategamefinderplayerstatus", request);
					}

					// When setting our status to Ready, we need to account for the case when the settings change during the ready RPC
					std::weak_ptr<PartyService> wThat = this->shared_from_this();
					return _gameFinderConnectionTask.then([request, wThat](pplx::task<void> task)
						{
							// I let GameFinder connection errors propagate as unspecified errors to the caller.
							// I don't see any good in creating a specific error type for these, since they most likely mean there's a bug
							task.wait();

							if (auto that = wThat.lock())
							{
								std::lock_guard<std::recursive_mutex> lg(that->_stateMutex);

								if (request.localSettingsVersion != that->_state.settings.settingsVersionNumber)
								{
									throw std::runtime_error(PartyError::Str::SettingsOutdated);
								}

								return that->_rpcService->rpc<void>("party.updategamefinderplayerstatus", request);
							}

							return pplx::task_from_result();
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
				pplx::task<void> promoteLeader(const std::string playerId)
				{
					return _rpcService->rpc<void>("party.promoteleader", playerId);
				}

				///
				/// Remove player from party this method can be call only by party leader.
				/// \param playerToKick is the user player id to be kicked
				pplx::task<void> kickPlayer(const std::string playerId)
				{
					return _rpcService->rpc<void>("party.kickplayer", playerId);
				}

				///
				/// Callback member
				///
				Event<Stormancer::GameFinder::GameFinderStatus> PartyGameFinderStateUpdated;
				Event<Stormancer::GameFinder::GameFinderResponse> onPartyGameFound;
				Event<MemberDisconnectionReason> LeftParty;
				Event<void> JoinedParty;
				Event<std::vector<PartyUserDto>> UpdatedPartyMembers;
				Event<PartySettings> UpdatedPartySettings;

				std::vector<PartyUserDto> members() const
				{
					std::lock_guard<std::recursive_mutex> lg(_stateMutex);
					return _state.members;
				}

				PartySettings settings() const
				{
					std::lock_guard<std::recursive_mutex> lg(_stateMutex);
					return _state.settings;
				}

				std::string leaderId() const
				{
					std::lock_guard<std::recursive_mutex> lg(_stateMutex);
					return _state.leaderId;
				}

				void initialize()
				{
					std::weak_ptr<PartyService> wThat = this->shared_from_this();
					auto scene = _scene.lock();
					auto rpcService = scene->dependencyResolver().resolve<RpcService>();

					rpcService->addProcedure("party.getPartyStateResponse", [wThat](RpcRequestContext_ptr ctx)
						{
							if (auto that = wThat.lock())
							{
								return that->handlePartyStateResponse(ctx);
							}
							return pplx::task_from_result();
						});

					rpcService->addProcedure("party.settingsUpdated", [wThat](RpcRequestContext_ptr ctx)
						{
							if (auto that = wThat.lock())
							{
								return that->handleSettingsUpdate(ctx);
							}
							return pplx::task_from_result();
						});

					rpcService->addProcedure("party.memberDataUpdated", [wThat](RpcRequestContext_ptr ctx)
						{
							if (auto that = wThat.lock())
							{
								return that->handleUserDataUpdate(ctx);
							}
							return pplx::task_from_result();
						});

					rpcService->addProcedure("party.memberStatusUpdated", [wThat](RpcRequestContext_ptr ctx)
						{
							if (auto that = wThat.lock())
							{
								return that->handleMemberStatusUpdate(ctx);
							}
							return pplx::task_from_result();
						});

					rpcService->addProcedure("party.memberConnected", [wThat](RpcRequestContext_ptr ctx)
						{
							if (auto that = wThat.lock())
							{
								return that->handleMemberConnected(ctx);
							}
							return pplx::task_from_result();
						});

					rpcService->addProcedure("party.memberDisconnected", [wThat](RpcRequestContext_ptr ctx)
						{
							if (auto that = wThat.lock())
							{
								return that->handleMemberDisconnected(ctx);
							}
							return pplx::task_from_result();
						});

					rpcService->addProcedure("party.leaderChanged", [wThat](RpcRequestContext_ptr ctx)
						{
							if (auto that = wThat.lock())
							{
								return that->handleLeaderChanged(ctx);
							}
							return pplx::task_from_result();
						});

					_scene.lock()->getConnectionStateChangedObservable().subscribe([wThat](ConnectionState state) {
						if (auto that = wThat.lock())
						{
							try
							{
								if (state == ConnectionState::Connected)
								{
									that->JoinedParty();
								}
								else if (state == ConnectionState::Disconnected)
								{
									that->_gameFinder->disconnectFromGameFinder(that->_state.settings.gameFinderName)
										.then([](pplx::task<void> t)
											{
												try {
													t.get();
												}
												catch (...) {}
											});

									MemberDisconnectionReason reason = MemberDisconnectionReason::Left;
									if (state.reason == "party.kicked")
									{
										reason = MemberDisconnectionReason::Kicked;
									}
									that->LeftParty(reason);
								}
							}
							catch (const std::exception& ex)
							{
								that->_logger->log(LogLevel::Error, "PartyService::ConnectionStateChanged", "An exception was thrown by a connection event handler", ex);
							}
						}

						});
				}

				pplx::task<void> waitForPartyReady()
				{
					return pplx::create_task(_partyStateReceived);
				}

			private:

				void updateGameFinder()
				{
					std::lock_guard<std::recursive_mutex> lg(_stateMutex);

					if (_currentGameFinder == _state.settings.gameFinderName)
					{
						return;
					}

					// This CTS prevents multiple game finder connection requests from queuing up.
					_gameFinderConnectionCts.cancel();
					_gameFinderConnectionCts = pplx::cancellation_token_source();

					// No need to wait for the old GF disconnection before connecting to the new GF
					_gameFinder->disconnectFromGameFinder(_currentGameFinder).then([](pplx::task<void> task)
						{
							try { task.wait(); }
							catch (...) {}
						});

					_currentGameFinder = _state.settings.gameFinderName;
					if (_currentGameFinder.empty())
					{
						return;
					}

					_logger->log(LogLevel::Trace, "PartyService", "Connecting to the party's GameFinder", _state.settings.gameFinderName);

					std::string newGameFinderName = _currentGameFinder;
					auto token = _gameFinderConnectionCts.get_token();
					std::weak_ptr<PartyService> wThat = this->shared_from_this();
					_gameFinderConnectionTask = _gameFinderConnectionTask.then([wThat, newGameFinderName, token](pplx::task<void> task)
						{
							// I want to recover from cancellation, but not from error, since error means we're leaving the party
							task.wait();

							auto that = wThat.lock();
							if (!that || token.is_canceled())
							{
								pplx::cancel_current_task();
							}

							return that->_gameFinder->connectToGameFinder(newGameFinderName);
						}, token)
						.then([wThat, newGameFinderName](pplx::task<void> task)
							{
								auto that = wThat.lock();
								try
								{
									auto status = task.wait();
									if (that && status == pplx::completed)
									{
										that->_logger->log(LogLevel::Trace, "PartyService", "Connected to the GameFinder", newGameFinderName);
									}
								}
								catch (const std::exception& ex)
								{
									if (that)
									{
										that->_logger->log(LogLevel::Error, "PartyService", "Error connecting to the GameFinder '" + newGameFinderName + "'", ex);
										if (auto scene = that->_scene.lock())
										{
											std::lock_guard<std::recursive_mutex> lg(that->_stateMutex);
											scene->disconnect().then([](pplx::task<void> t) { try { t.get(); } catch (...) {}});
											that->_scene.reset();
										}
									}
									throw;
								}
							}, token);
				}

				bool checkVersionNumber(RpcRequestContext_ptr ctx)
				{
					auto versionNumber = ctx->readObject<int>();
					if (_state.version > 0 && versionNumber == _state.version + 1)
					{
						_state.version = versionNumber;
						return true;
					}
					else
					{
						_logger->log(LogLevel::Trace, "PartyService::checkVersionNumber", "Version number mismatch ; current=" + std::to_string(_state.version) + ", received=" + std::to_string(versionNumber));
						syncPartyState();
						return false;
					}
				}

				// This returns void because we must not block on it (or else we would cause a timeout in party update RPC)
				void syncPartyState()
				{
					if (_isPendingStateSync)
					{
						return;
					}
					_isPendingStateSync = true;
					auto logger = _logger;
					std::weak_ptr<PartyService> wThat = this->shared_from_this();
					_rpcService->rpc("party.getpartystate").then([logger, wThat](pplx::task<void> task)
						{
							bool success = true;
							try
							{
								success = task.wait() == pplx::completed;
							}
							catch (const std::exception& ex)
							{
								logger->log(LogLevel::Error, "PartyService::syncPartyState", "Error in party.getpartystate RPC", ex);
								success = false;
							}
							// In case of success, _isPendingStateSync is reset in the party.getPartyStateResponse handler.
							if (!success)
							{
								if (auto that = wThat.lock())
								{
									std::lock_guard<std::recursive_mutex> lg(that->_stateMutex);
									that->_isPendingStateSync = false;
								}
							}
						});
				}

				pplx::task<void> handlePartyStateResponse(RpcRequestContext_ptr ctx)
				{
					std::lock_guard<std::recursive_mutex> lg(_stateMutex);

					_state = ctx->readObject<PartyState>();
					_logger->log(LogLevel::Trace, "PartyService::handlePartyStateResponse", "Received party state, version = " + std::to_string(_state.version));

					updateLeader();
					updateGameFinder();
					_partyStateReceived.set();
					_isPendingStateSync = false;
					this->UpdatedPartySettings(_state.settings);
					this->UpdatedPartyMembers(_state.members);

					return pplx::task_from_result();
				}

				pplx::task<void> handleSettingsUpdate(RpcRequestContext_ptr ctx)
				{
					std::lock_guard<std::recursive_mutex> lg(_stateMutex);

					if (checkVersionNumber(ctx))
					{
						_logger->log(LogLevel::Trace, "PartyService::handleSettingsUpdate", "Received settings update, version = " + std::to_string(_state.version));
						_state.settings = ctx->readObject<PartySettingsInternal>();
						updateGameFinder();
						this->UpdatedPartySettings(_state.settings);
					}

					return pplx::task_from_result();
				}

				pplx::task<void> handleUserDataUpdate(RpcRequestContext_ptr ctx)
				{
					std::lock_guard<std::recursive_mutex> lg(_stateMutex);

					if (checkVersionNumber(ctx))
					{
						_logger->log(LogLevel::Trace, "PartyService::handleUserDataUpdate", "Received user data update, version = " + std::to_string(_state.version));
						auto update = ctx->readObject<PartyUserData>();
						auto member = std::find_if(_state.members.begin(), _state.members.end(), [&update](const PartyUserDto& user) { return update.userId == user.userId; });

						if (member != _state.members.end())
						{
							member->userData = update.userData;
							this->UpdatedPartyMembers(_state.members);
						}
					}

					return pplx::task_from_result();
				}

				pplx::task<void> handleMemberStatusUpdate(RpcRequestContext_ptr ctx)
				{
					std::lock_guard<std::recursive_mutex> lg(_stateMutex);

					if (checkVersionNumber(ctx))
					{
						_logger->log(LogLevel::Trace, "PartyService::handleMemberStatusUpdate", "Received member status update, version = " + std::to_string(_state.version));

						auto updates = ctx->readObject<BatchStatusUpdate>();
						for (const auto& update : updates.memberStatus)
						{
							auto member = std::find_if(_state.members.begin(), _state.members.end(), [&update](const PartyUserDto& user) { return update.userId == user.userId; });

							if (member != _state.members.end())
							{
								member->partyUserStatus = update.status;
							}
						}

						this->UpdatedPartyMembers(_state.members);
					}

					return pplx::task_from_result();
				}

				pplx::task<void> handleMemberConnected(RpcRequestContext_ptr ctx)
				{
					std::lock_guard<std::recursive_mutex> lg(_stateMutex);

					if (checkVersionNumber(ctx))
					{
						auto member = ctx->readObject<PartyUserDto>();
						_logger->log(LogLevel::Trace, "PartyService::handleMemberConnected", "New party member: Id=" + member.userId + ", version = " + std::to_string(_state.version));

						_state.members.push_back(member);
						this->UpdatedPartyMembers(_state.members);
					}

					return pplx::task_from_result();
				}

				pplx::task<void> handleMemberDisconnected(RpcRequestContext_ptr ctx)
				{
					std::lock_guard<std::recursive_mutex> lg(_stateMutex);

					if (checkVersionNumber(ctx))
					{
						auto message = ctx->readObject<MemberDisconnection>();
						_logger->log(LogLevel::Trace, "PartyService::handleMemberDisconnected", "Member disconnected: Id=" + message.userId + ", Reason=" + std::to_string(static_cast<int>(message.reason)) + ", version = " + std::to_string(_state.version));

						auto member = std::find_if(_state.members.begin(), _state.members.end(), [&message](const PartyUserDto& user) { return message.userId == user.userId; });
						if (member != _state.members.end())
						{
							_state.members.erase(member);
							this->UpdatedPartyMembers(_state.members);
						}
					}

					return pplx::task_from_result();
				}

				pplx::task<void> handleLeaderChanged(RpcRequestContext_ptr ctx)
				{
					std::lock_guard<std::recursive_mutex> lg(_stateMutex);

					if (checkVersionNumber(ctx))
					{
						auto leaderId = ctx->readObject<std::string>();
						_logger->log(LogLevel::Trace, "PartyService::handleLeaderChanged", "New leader: Id=" + leaderId + ", version = " + std::to_string(_state.version));

						if (_state.leaderId != leaderId)
						{
							_state.leaderId = leaderId;
							updateLeader();
							this->UpdatedPartyMembers(_state.members);
						}
					}

					return pplx::task_from_result();
				}

				void updateLeader()
				{
					const std::string& newLeaderId = _state.leaderId;
					auto currentLeader = std::find_if(_state.members.begin(), _state.members.end(), [](const PartyUserDto& user) { return user.isLeader; });
					if (currentLeader != _state.members.end())
					{
						currentLeader->isLeader = false;
					}

					auto newLeader = std::find_if(_state.members.begin(), _state.members.end(), [&newLeaderId](const PartyUserDto& user) { return newLeaderId == user.userId; });
					if (newLeader != _state.members.end())
					{
						newLeader->isLeader = true;
					}
				}

				PartyState _state;
				std::string _currentGameFinder;
				std::weak_ptr<Scene> _scene;
				std::shared_ptr<ILogger> _logger;
				std::shared_ptr<RpcService> _rpcService;
				std::shared_ptr<Stormancer::GameFinder::GameFinderApi> _gameFinder;

				std::string _myUserId;
				// Synchronize async state update, as well as getters.
				// This is "coarse grain" synchronization, but the simplicity gains vs. multiple mutexes win against the possible performance loss imo.
				mutable std::recursive_mutex _stateMutex;
				// Prevent having multiple game finder connection tasks at the same time (could happen if multiple settings updates are received in a short amount of time)
				pplx::task<void> _gameFinderConnectionTask = pplx::task_from_result();
				pplx::cancellation_token_source _gameFinderConnectionCts;
				// Used to signal to client code when the party is ready
				pplx::task_completion_event<void> _partyStateReceived;
				bool _isPendingStateSync = false;
			};

			class PartyContainer
			{
				friend class Party_Impl;
			public:
				PartyContainer(
					std::shared_ptr<Scene> scene,
					Event<MemberDisconnectionReason>::Subscription LeftPartySubscription,
					Event<std::vector<PartyUserDto>>::Subscription UpdatedPartyMembersSubscription,
					Event<PartySettings>::Subscription UpdatedPartySettingsSubscription
				)
					: _partyScene(scene)
					, _partyService(scene->dependencyResolver().resolve<PartyService>())
					, LeftPartySubscription(LeftPartySubscription)
					, UpdatedPartyMembersSubscription(UpdatedPartyMembersSubscription)
					, UpdatedPartySettingsSubscription(UpdatedPartySettingsSubscription)
				{
				}

				PartySettings settings() const
				{
					return  _partyService->settings();
				}

				std::vector<PartyUserDto> members() const
				{
					return _partyService->members();
				}

				bool isLeader() const
				{
					return (_partyService->leaderId() == _partyScene->dependencyResolver().resolve<Stormancer::Users::UsersApi>()->userId());
				}

				std::string leaderId() const
				{
					return _partyService->leaderId();
				}

				std::shared_ptr<Scene> getScene() const { return _partyScene; }
				std::string id() const { return _partyScene->id(); }

				bool registerInvitationRequest(std::string recipientId, InvitationRequest& request)
				{
					std::lock_guard<std::mutex> lg(_invitationsMutex);
					auto it = _pendingInvitationRequests.find(recipientId);
					request = _pendingInvitationRequests[recipientId];
					if (it == _pendingInvitationRequests.end())
					{


						return true;
					}
					else
					{


						return false;

					}
				}

				void closeInvitationRequest(std::string recipientId)
				{
					std::lock_guard<std::mutex> lg(_invitationsMutex);

					if (_pendingInvitationRequests.find(recipientId) != _pendingInvitationRequests.end())
					{
						_pendingInvitationRequests[recipientId].cts.cancel();
						_pendingInvitationRequests.erase(recipientId);
					}
					/*else //No invitation is successfully canceled.
					{
						throw std::runtime_error(&("Could not cancel a party invitation as there none pending for " + recipientId)[0]);
					}*/
				}

				~PartyContainer()
				{
					std::lock_guard<std::mutex> lg(_invitationsMutex);

					for (auto& request : _pendingInvitationRequests)
					{
						request.second.cts.cancel();
					}
				}

			private:
				std::shared_ptr<Scene> _partyScene;
				std::shared_ptr<PartyService> _partyService;

				Event<MemberDisconnectionReason>::Subscription LeftPartySubscription;
				Event<std::vector<PartyUserDto>>::Subscription UpdatedPartyMembersSubscription;
				Event<PartySettings>::Subscription UpdatedPartySettingsSubscription;

				std::unordered_map<std::string, InvitationRequest> _pendingInvitationRequests;
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
					std::weak_ptr<Stormancer::Users::UsersApi> users,
					std::weak_ptr<ILogger> logger,
					std::shared_ptr<IActionDispatcher> dispatcher,
					std::vector<std::shared_ptr<IPartyEventHandler>> eventHandlers,
					std::shared_ptr<GameFinder::GameFinderApi> gameFinder
				)
					: ClientAPI(users)
					, _logger(logger)
					, _dispatcher(dispatcher)
					, _eventHandlers(eventHandlers)
					, _gameFinder(gameFinder)
				{}

				pplx::task<void> createParty(const PartyRequestDto& partySettings) override
				{
					if (_party)
					{
						return pplx::task_from_exception<void>(std::runtime_error(PartyError::Str::AlreadyInParty));
					}
					auto users = _users.lock();
					if (!users)
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
							if (auto partyManagment = wPartyManagement.lock())
							{
								return partyManagment->getPartySceneByToken(token);
							}
							throw std::runtime_error("destroyed");
						}, _dispatcher)
						.then([wPartyManagement](pplx::task<std::shared_ptr<PartyContainer>> task)
							{
								try
								{
									return task.get();
								}
								catch (...)
								{
									if (auto that = wPartyManagement.lock())
									{
										that->_party = nullptr;
									}
									throw;
								}
							}, _dispatcher);

						auto userTask = partyTask.then([wPartyManagement](std::shared_ptr<PartyContainer>)
							{
								if (auto that = wPartyManagement.lock())
								{
									// Wait for the party task to be complete before triggering these events, to stay consistent with isInParty()
									that->_onJoinedParty();
									that->_onUpdatedPartyMembers(that->getPartyMembers());
									that->_onUpdatedPartySettings(that->getPartySettings());
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
					return _users.lock()->getSceneConnectionToken("stormancer.plugins.party", invitation.SceneId, pplx::cancellation_token::none())
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

				PartyUserDto getLocalMember() const override
				{
					if (!isInParty())
					{
						throw std::runtime_error(PartyError::Str::NotInParty);
					}

					auto myId = _users.lock()->userId();
					auto members = _party->get()->members();
					auto it = std::find_if(members.begin(), members.end(), [&myId](const PartyUserDto& user)
						{
							return user.userId == myId;
						});

					if (it != members.end())
					{
						return *it;
					}
					assert(false); // Bug!
					throw std::runtime_error(PartyError::Str::NotInParty);
				}

				PartySettings getPartySettings() const override
				{
					if (!isInParty())
					{
						throw std::runtime_error(PartyError::Str::NotInParty);
					}

					return _party->get()->settings();
				}

				std::string getPartyLeaderId() const override
				{
					if (!isInParty())
					{
						throw std::runtime_error(PartyError::Str::NotInParty);
					}

					return _party->get()->leaderId();
				}

				bool isLeader() const override
				{
					if (!isInParty())
					{
						throw std::runtime_error(PartyError::Str::NotInParty);
					}

					return _party->get()->isLeader();
				}

				std::vector<std::string> getSentPendingInvitations()
				{
					std::vector<std::string> pendingInvitations;
					if (!_party || !_party->is_done())
					{
						return pendingInvitations;
					}



					std::lock_guard<std::recursive_mutex> lg(_invitationsMutex);
					for (const auto& it : _party->get()->_pendingInvitationRequests)
					{
						pendingInvitations.push_back(it.first);
					}
					return pendingInvitations;

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
							return partyService->updatePlayerStatus(playerStatus);
						});
				}

				pplx::task<void> updatePartySettings(PartySettings partySettingsDto) override
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

				pplx::task<void> promoteLeader(std::string userId) override
				{
					if (!isInParty())
					{
						return pplx::task_from_exception<void>(std::runtime_error(PartyError::Str::NotInParty));
					}

					return _party->then([userId](std::shared_ptr<PartyContainer> party) {
						std::shared_ptr<PartyService> partyService = party->getScene()->dependencyResolver().resolve<PartyService>();
						return partyService->promoteLeader(userId);
						});
				}

				pplx::task<void> kickPlayer(std::string userId) override
				{
					if (!isInParty())
					{
						return pplx::task_from_exception<void>(std::runtime_error(PartyError::Str::NotInParty));
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

					auto wUsers = _users;
					auto wThat = this->weak_from_this();
					return _party->then([wUsers, wThat, recipient, ct](std::shared_ptr<PartyContainer> party)
						{
							auto users = wUsers.lock();
							auto that = wThat.lock();
							if (!users || !that)
							{
								return pplx::task_from_result();
							}

							auto partyId = party->id();


							InvitationRequest request;
							auto newRequest = party->registerInvitationRequest(recipient, request);

							std::weak_ptr<PartyContainer> wParty(party);
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


							if (!newRequest)
							{
								return request.task;
							}
							else
							{


								auto requestTask = users->sendRequestToUser<void>(recipient, "party.invite", request.cts.get_token(), partyId)
									.then([recipient, wParty]
										{
											if (auto party = wParty.lock())
											{
												party->closeInvitationRequest(recipient);
											}
										});
								request.task = requestTask;
								return requestTask;
							}

						});
				}

				pplx::task<void> cancelPartyInvitation(std::string recipient)
				{
					return _party->then([recipient](std::shared_ptr<PartyContainer> party)
						{
							party->closeInvitationRequest(recipient);
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

				Event<void>::Subscription subscribeOnJoinedParty(std::function<void()> callback) override
				{
					return _onJoinedParty.subscribe(callback);
				}

				Event<MemberDisconnectionReason>::Subscription subscribeOnLeftParty(std::function<void(MemberDisconnectionReason)> callback) override
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

				Subscription subscribeOnGameFinderStatusUpdate(std::function<void(PartyGameFinderStatus)> callback) override
				{
					return _onGameFinderStatusUpdate.subscribe(callback);
				}

				Subscription subscribeOnGameFound(std::function<void(GameFinder::GameFinderResponse)> callback) override
				{
					return _onGameFound.subscribe(callback);
				}

				Subscription subscribeOnGameFinderFailure(std::function<void(PartyGameFinderFailure)> callback) override
				{
					return _onGameFinderFailure.subscribe(callback);
				}


				void initialize()
				{
					auto wThat = this->weak_from_this();
					_users.lock()->setOperationHandler("party.invite", [wThat](Stormancer::Users::OperationCtx& ctx)
						{
							if (auto that = wThat.lock())
							{
								return that->invitationHandler(ctx);
							}
							return pplx::task_from_result();
						});
					_gameFinder->subsribeGameFinderStateChanged([wThat](GameFinder::GameFinderStatusChangedEvent evt)
						{
							if (auto that = wThat.lock())
							{
								if (that->getPartySettings().gameFinderName == evt.gameFinder)
								{
									switch (evt.status)
									{
									case GameFinder::GameFinderStatus::Searching:
										that->_onGameFinderStatusUpdate(PartyGameFinderStatus::SearchInProgress);
										break;
									default:
										that->_onGameFinderStatusUpdate(PartyGameFinderStatus::SearchStopped);
										break;
									}
								}
							}
						});
					_gameFinder->subsribeGameFound([wThat](GameFinder::GameFoundEvent evt)
						{
							if (auto that = wThat.lock())
							{
								if (that->getPartySettings().gameFinderName == evt.gameFinder)
								{
									that->_onGameFound(evt.data);
								}
							}
						});
					_gameFinder->subscribeFindGameFailed([wThat](GameFinder::FindGameFailedEvent evt)
						{
							if (auto that = wThat.lock())
							{
								if (that->getPartySettings().gameFinderName == evt.gameFinder)
								{
									that->_onGameFinderFailure(PartyGameFinderFailure{ evt.reason });
								}
							}
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
				Event<void> _onJoinedParty;
				Event<MemberDisconnectionReason> _onLeftParty;
				Event<PartyInvitation> _onInvitationReceived;
				Event<std::string> _onInvitationCanceled;
				Event<PartyGameFinderStatus> _onGameFinderStatusUpdate;
				Event<GameFinder::GameFinderResponse> _onGameFound;
				Event<PartyGameFinderFailure> _onGameFinderFailure;

				pplx::task<std::shared_ptr<PartyContainer>> getPartySceneByToken(const std::string& token)
				{
					auto users = _users.lock();
					auto wThat = this->weak_from_this();

					return users->connectToPrivateSceneByToken(token).then([wThat](std::shared_ptr<Scene> scene)
						{
							auto that = wThat.lock();
							if (!that)
							{
								throw PointerDeletedException("PartyApi");
							}
							return that->initPartyFromScene(scene);
						})
						.then([wThat](std::shared_ptr<PartyContainer> container)
							{
								auto that = wThat.lock();
								if (!that)
								{
									throw PointerDeletedException("PartyApi");
								}

								pplx::task<void> handlersTask = pplx::task_from_result();
								for (auto handler : that->_eventHandlers)
								{
									handlersTask = handlersTask.then([wThat, handler, container]
										{
											if (auto that = wThat.lock())
											{
												return handler->onJoiningParty(that, container->id());
											}
											throw PointerDeletedException("PartyApi");
										}, that->_dispatcher);
								}

								return handlersTask.then([container](pplx::task<void> task)
									{
										try
										{
											task.get();
											return container;
										}
										catch (...)
										{
											// Keep container alive so that OnLeftParty gets triggered for event handlers
											container->getScene()->disconnect().then([container](pplx::task<void> t) { try { t.wait(); } catch (...) {}});
											throw;
										}
									});
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

					auto sceneId = scene->id();

					auto party = std::make_shared<PartyContainer>(
						scene,
						partyService->LeftParty.subscribe([wPartyManagement, sceneId](MemberDisconnectionReason reason) {
							if (auto partyManagement = wPartyManagement.lock())
							{
								pplx::task<void> handlersTask = pplx::task_from_result();
								auto logger = partyManagement->_logger;
								for (auto handler : partyManagement->_eventHandlers)
								{
									// Capture a shared_ptr because the handlers could do important cleanup and need access to PartyApi
									handlersTask = handlersTask.then([partyManagement, sceneId, handler]
										{
											return handler->onLeavingParty(partyManagement, sceneId);
										}).then([logger](pplx::task<void> task)
											{
												// As these handlers could do important cleanup (e.g leaving a session), it is important that we run all of them even if some fail
												// This is why I handle errors for each of them
												try
												{
													task.wait();
												}
												catch (const std::exception& ex)
												{
													logger->log(LogLevel::Error, "Party_Impl::OnLeftParty", "An exception was thrown by an onLeavingParty handler", ex);
												}
											});
								}

								if (partyManagement->isInParty())
								{
									partyManagement->_party = nullptr;
									partyManagement->_onLeftParty(reason);
								}
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

				pplx::task<void> invitationHandler(Stormancer::Users::OperationCtx& ctx)
				{
					Serializer serializer;
					auto senderId = ctx.originId;
					auto sceneId = serializer.deserializeOne<std::string>(ctx.request->inputStream());
					_logger->log(LogLevel::Trace, "Party_Impl::invitationHandler", "Received an invitation: sender=" + senderId + " ; sceneId=" + sceneId);

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
										that->_logger->log(LogLevel::Trace, "Party_Impl::invitationHandler", "Sender (id=" + senderId + ") canceled an invitation");
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
				std::vector<std::shared_ptr<IPartyEventHandler>> _eventHandlers;
				std::shared_ptr<Stormancer::GameFinder::GameFinderApi> _gameFinder;
			};
		}

		class PartyPlugin : public IPlugin
		{
			static constexpr const char* PARTY_VERSION = "2019-08-30.1";
			static constexpr const char* PARTYMANAGEMENT_VERSION = "2019-08-30.1";

			void registerSceneDependencies(ContainerBuilder& builder, std::shared_ptr<Scene> scene) override
			{
				auto version = scene->getHostMetadata("stormancer.party");
				if (version == PARTY_VERSION)
				{
					builder.registerDependency<details::PartyService, Scene>().singleInstance();
				}

				version = scene->getHostMetadata("stormancer.partymanagement");
				if (version == PARTYMANAGEMENT_VERSION)
				{
					builder.registerDependency<details::PartyManagementService, Scene>().singleInstance();
				}
			}

			void sceneCreated(std::shared_ptr<Scene> scene) override
			{
				if (scene->getHostMetadata("stormancer.party") == PARTY_VERSION)
				{
					scene->dependencyResolver().resolve<details::PartyService>()->initialize();
				}
			}

			void registerClientDependencies(ContainerBuilder& builder) override
			{
				builder.registerDependency<PartyApi>([](const DependencyScope& dr) {
					auto partyImpl = std::make_shared<details::Party_Impl>(
						dr.resolve<Stormancer::Users::UsersApi>(),
						dr.resolve<ILogger>(),
						dr.resolve<IActionDispatcher>(),
						dr.resolveAll<IPartyEventHandler>(),
						dr.resolve<GameFinder::GameFinderApi>()
						);
					// initialize() needs weak_from_this(), so it can't be called from Party_Impl's constructor
					partyImpl->initialize();
					return partyImpl;
					}).singleInstance();
			}
		};
	}
}

MSGPACK_ADD_ENUM(Stormancer::Party::PartyUserStatus)
MSGPACK_ADD_ENUM(Stormancer::Party::MemberDisconnectionReason)
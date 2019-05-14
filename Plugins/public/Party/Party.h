#pragma once
#include "Party/PartyModels.h"
#include "stormancer/Event.h"
#include "stormancer/Tasks.h"

namespace Stormancer
{
	class PartyContainer;
	
	class PartyInvitations;

	class Party
	{
	public:
		/// <summary>
		/// Virtual destructor
		/// </summary>
		virtual ~Party() {}

		/// <summary>
		/// Join a party using a platform session Id (steam/psn/xboxlive) session id as key
		/// </summary>
		/// <returns>A task that completes with the operation.</returns>
		virtual pplx::task<std::shared_ptr<PartyContainer>> joinPartySceneByPlatformSessionId(const std::string uniqueOnlinePartyName) = 0;
		
		/// <summary>
		/// Join a party using a connection token provided by the server
		/// </summary>
		/// <returns>A task that completes with the operation.</returns>
		virtual pplx::task<std::shared_ptr<PartyContainer>> joinPartySceneByConnectionToken(const std::string& connectionToken) = 0;

		/// <summary>
		/// Leave the party
		/// </summary>
		/// <returns>A task that completes with the operation.</returns>
		virtual pplx::task<void> leaveParty() = 0;

		/// <summary>
		/// Get the current party
		/// </summary>
		/// <returns>the party or an empty pointer</returns>
		virtual pplx::task<std::shared_ptr<PartyContainer>> getParty() = 0;

		/// <summary>
		/// Set the player status.
		/// When all players in party are ready, gamefinding is automatically started.
		/// </summary>
		/// <param name="playerStatus">Ready or not ready</param>
		/// <returns></returns>
		virtual pplx::task<void> updatePlayerStatus(PartyUserStatus playerStatus) = 0;

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
		virtual pplx::task<void> updatePartySettings(PartySettingsDto partySettings) = 0;

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
		virtual pplx::task<bool> PromoteLeader(std::string userId) = 0;

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
		/// Create a party and set the player as leader
		/// </summary>
		/// <remarks>
		/// If the player is currently in a party, the operation fails.
		/// </remarks>
		/// <param name="partyRequest">Party creation parameters</param>
		/// <returns></returns>
		virtual pplx::task<std::shared_ptr<PartyContainer>> createParty(const  PartyRequestDto& partyRequest) = 0;

		/// <summary>
		/// Send an invitation to the specified userId to join the party.
		/// </summary>
		/// <param name="userId">The id of the target user</param>
		/// <returns></returns>
		virtual pplx::task<void> sendInvitation(const std::string& userId) = 0;

		virtual Event<PartySettings>::Subscription subscribeOnUpdatedPartySettings(std::function<void(PartySettings)> callback) = 0;
		virtual Event<std::vector<PartyUserDto>>::Subscription subscribeOnUpdatedPartyMembers(std::function<void(std::vector<PartyUserDto>)> callback) = 0;
		virtual Event<PartyUserData>::Subscription subscribeOnUpdatedUserData(std::function<void(PartyUserData)> callback) = 0;
		virtual Event<void>::Subscription subscribeOnJoinedParty(std::function<void()> callback) = 0;
		virtual Event<void>::Subscription subscribeOnKickedFromParty(std::function<void()> callback) = 0;
		virtual Event<void>::Subscription subscribeOnLeftParty(std::function<void()> callback) = 0;	
		
		/// <summary>
		/// Get pending party invitations for the player.
		/// </summary>
		/// <returns></returns>
		virtual PartyInvitations& getPartyInvitation() = 0;
	};
}
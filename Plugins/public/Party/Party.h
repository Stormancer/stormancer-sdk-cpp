#pragma once
#include "Party/PartyModels.h"

namespace Stormancer
{
	class PartyContainer;
	class Subscription;
	// We need to implement new 
	class PartyInvitations;

	class Party
	{
	public:
		/// <summary>
		/// Virtual destructor
		/// </summary>
		virtual ~Party() {}

		virtual pplx::task<std::shared_ptr<PartyContainer>> joinPartySceneByPlatformSessionId(const std::string uniqueOnlinePartyName) = 0;
		virtual pplx::task<std::shared_ptr<PartyContainer>> joinPartySceneByConnectionToken(const std::string& connectionToken) = 0;

		/// <summary>
		/// Leave the party. The client disconnect the party scene.
		/// </summary>
		/// <returns></returns>
		virtual pplx::task<void> leaveParty() = 0;

		/// <summary>
		/// Get the current party or party is pending connection.
		/// </summary>
		/// <returns>Return pointer to shared if the client is connected or throw a exception</returns>
		virtual pplx::task<std::shared_ptr<PartyContainer>> getParty() = 0;

		/// <summary>
		/// Update if the player are ready to start game finder or not.
		/// If all players in party are ready the gamefinder action is triggered on server.
		/// </summary>
		/// <param name="playerStatus">Ready or not ready</param>
		/// <returns></returns>
		virtual pplx::task<void> updatePlayerStatus(PartyUserStatus playerStatus) = 0;

		/// <summary>
		/// Update the party data on server and on each client. Only the party leader can update the party settings
		/// To get updated party settings you could subscribe to subscribeOnUpdatedPartySettings.
		/// </summary>
		/// <param name="partySettings"></param>
		/// <returns></returns>
		virtual pplx::task<void> updatePartySettings(PartySettingsDto partySettings) = 0;
		virtual pplx::task<void> updatePlayerData(std::string data) = 0;
		virtual pplx::task<bool> PromoteLeader(std::string userId) = 0;
		virtual pplx::task<bool> kickPlayer(std::string userId) = 0;
		virtual pplx::task<std::shared_ptr<PartyContainer>> createParty(const  PartyRequestDto& partyRequestDto) = 0;

		virtual pplx::task<void> sendInvitation(const std::string& id) = 0;

		virtual Event<PartySettings>::Subscription subscribeOnUpdatedPartySettings(std::function<void(PartySettings)> callback) = 0;
		virtual Event<std::vector<PartyUserDto>>::Subscription subscribeOnUpdatedPartyMembers(std::function<void(std::vector<PartyUserDto>)> callback) = 0;
		virtual Event<PartyUserData>::Subscription subscribeOnUpdatedUserData(std::function<void(PartyUserData)> callback) = 0;
		virtual Event<void>::Subscription subscribeOnJoinedParty(std::function<void()> callback) = 0;
		virtual Event<void>::Subscription subscribeOnKickedFromParty(std::function<void()> callback) = 0;
		virtual Event<void>::Subscription subscribeOnLeftParty(std::function<void()> callback) = 0;	
		
		virtual PartyInvitations& getPartyInvitation() = 0;
	};
}
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

		virtual ~Party() {}

		virtual pplx::task<std::shared_ptr<PartyContainer>> joinPartySceneByPlatformSessionId(const std::string uniqueOnlinePartyName) = 0;
		virtual pplx::task<std::shared_ptr<PartyContainer>> joinPartySceneByConnectionToken(const std::string& connectionToken) = 0;
		virtual pplx::task<void> leaveParty() = 0;
		virtual pplx::task<std::shared_ptr<PartyContainer>> getParty() = 0;


		virtual pplx::task<void> updatePlayerStatus(PartyUserStatus playerStatus) = 0;
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
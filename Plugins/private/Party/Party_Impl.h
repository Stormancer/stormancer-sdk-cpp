#pragma once
#include "core/ClientAPI.h"

#include "stormancer/Event.h"
#include "Party/Party.h"
#include "Party/PartyModels.h"
#include "PartyManagementService.h"
#include "Authentication/AuthenticationService.h"

namespace Stormancer
{
	///
	///Forward declare
	class GameFinder;

	class Party_Impl : public ClientAPI<Party_Impl>, public Party
	{
	public:
		Party_Impl(std::weak_ptr<AuthenticationService> auth, std::weak_ptr<ILogger> logger, std::weak_ptr<GameFinder> gameFinder);

		pplx::task<std::shared_ptr<PartyContainer>> joinPartySceneByPlatformSessionId(const std::string uniqueOnlinePartyName) override;
		pplx::task<std::shared_ptr<PartyContainer>> joinPartySceneByConnectionToken(const std::string& connectionToken) override;
		pplx::task<void> leaveParty() override;
		pplx::task<std::shared_ptr<PartyContainer>> getParty() override;


		pplx::task<void> updatePlayerStatus(PartyUserStatus playerStatus) override;
		pplx::task<void> updatePartySettings(PartySettingsDto partySettings) override;
		pplx::task<void> updatePlayerData(std::string data) override;
		pplx::task<bool> PromoteLeader(std::string userId) override;
		pplx::task<bool> kickPlayer(std::string userId) override;
		pplx::task<std::shared_ptr<PartyContainer>> createParty(const  PartyRequestDto& partyRequestDto) override;

		pplx::task<void> sendInvitation(const std::string& id) override;

		Event<PartySettings>::Subscription subscribeOnUpdatedPartySettings(std::function<void(PartySettings)> callback) override;
		Event<std::vector<PartyUserDto>>::Subscription subscribeOnUpdatedPartyMembers(std::function<void(std::vector<PartyUserDto>)> callback) override;
		Event<PartyUserData>::Subscription subscribeOnUpdatedUserData(std::function<void(PartyUserData)> callback) override;
		Event<void>::Subscription subscribeOnJoinedParty(std::function<void()> callback) override;
		Event<void>::Subscription subscribeOnKickedFromParty(std::function<void()> callback) override;
		Event<void>::Subscription subscribeOnLeftParty(std::function<void()> callback) override;

		PartyInvitations& getPartyInvitation() override 
		{
			return invitations;
		}
		
		void initialize();
	private:

		PartyInvitations invitations;
		// Events
		Event<PartySettings> _onUpdatedPartySettings;
		Event<std::vector<PartyUserDto>> _onUpdatedPartyMembers;
		Event<PartyUserData> _onUpdatedUserData;
		Event<void> _onJoinedParty;
		Event<void> _onKickedFromParty;
		Event<void> _onLeftParty;

		pplx::task<std::shared_ptr<PartyContainer>> getPartySceneByOnlinePartyName(const std::string uniqueOnlinePartyName);
		pplx::task<std::shared_ptr<PartyContainer>> getPartySceneByToken(const std::string& connectionToken);

		std::shared_ptr<PartyContainer> initPartyFromScene(std::shared_ptr<Scene> scene);
		
		std::weak_ptr<ILogger> _logger;
		std::weak_ptr<GameFinder> _gameFinder;
		rxcpp::subscription _partySceneConnectionStateSubscription;

		bool listened;
		std::string _uniqueOnlinePartyName;
		std::shared_ptr<pplx::task<std::shared_ptr<PartyContainer>>> _party;

		pplx::task<std::shared_ptr<PartyManagementService>> getPartyManagementService();

		
	};
}
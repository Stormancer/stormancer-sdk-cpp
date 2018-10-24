#pragma once

#include "stormancer/Action2.h"
#include "Party/PartyService.h"

namespace Stormancer
{
	struct PartyRequestDto
	{
		std::string PlatformSessionId;
		u_int64 PartySize;
		std::string GameFinderName;
		MSGPACK_DEFINE(PlatformSessionId, PartySize, GameFinderName);
	};
	class AuthenticationService;
	class ILogger;
	class GameFinder;

	class PartyManagement : public std::enable_shared_from_this<PartyManagement>
	{
	public:
		PartyManagement(std::weak_ptr<AuthenticationService> auth, std::weak_ptr<ILogger> logger, std::weak_ptr<GameFinder> gameFinder);

		pplx::task<Party_ptr> joinPartyScene(const std::string uniqueOnlinePartyName);
		pplx::task<void> leaveParty();
		pplx::task<Party_ptr> getParty();


		pplx::task<void> updatePlayerStatus(PartyUserStatus playerStatus);
		pplx::task<void> updatePartySettings(PartySettingsDto partySettings);
		pplx::task<void> updatePlayerData(std::string data);
		pplx::task<bool> PromoteLeader(std::string userId);
		pplx::task<bool> KickPlayer(std::string userId);
		pplx::task<Stormancer::Party_ptr> CreateParty(const  PartyRequestDto& partyRequestDto);

		Action2<PartySettings> onUpdatedPartySettings;

		Action2<std::vector<PartyUserDto>> onUpdatedPartyMembers;
		Action2<PartyUserData> onUpdatedUserData;
		Action2<void> onJoinedParty;
		Action2<void> onLeftParty;
	private:
		

		pplx::task<Party_ptr> getPartyScene(const std::string uniqueOnlinePartyName);
		void addParty(const std::string uniqueOnlinePartyName, const Party_ptr party);

		///
		/// User Exposed callback




		std::weak_ptr<AuthenticationService> _auth;
		std::weak_ptr<ILogger> _logger;
		std::weak_ptr<GameFinder> _gameFinder;

		bool listened;
		std::string _uniqueOnlinePartyName;
		std::shared_ptr<pplx::task<Party_ptr>> _party;

	};

	class PartyManagementService
	{

	public:

		PartyManagementService(Scene_ptr scene);

		pplx::task<std::string> CreateParty(const  PartyRequestDto& partyRequestDto);

		void onPlateformSessionCreated(std::function<void(std::string onlineSessionId)> callback);

	private:
		std::shared_ptr<ILogger> _logger;
		std::weak_ptr<Scene> _scene;

		//Internal callbacks
		std::function<void(RpcRequestContext_ptr requestContext)> _onCreateSession;
	};
}
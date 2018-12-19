#pragma once
#include "Core/ClientAPI.h"
#include "stormancer/Event.h"
#include "Party/PartyService.h"

namespace Stormancer
{
	struct PartyRequestDto
	{
		std::string PlatformSessionId;
		uint64 PartySize;
		std::string GameFinderName;
		std::string CustomData;
		bool StartOnlyIfPartyFull;
		MSGPACK_DEFINE(PlatformSessionId, PartySize, GameFinderName, CustomData, StartOnlyIfPartyFull);
	};
	class AuthenticationService;
	class ILogger;
	class GameFinder;
	class PartyManagementService;


	class PartyInvitation
	{
	public:

		std::string UserId;
		std::string SceneId;
		PartyInvitation(std::string userId, std::string sceneId)
		{
			UserId = userId;
			SceneId = sceneId;
		}

		Event<bool> onAnswer;

	};

	class PartyInvitations : public std::enable_shared_from_this<PartyInvitations>
	{
	public:

		///
		/// Sent to server the new party status
		///
		std::shared_ptr<PartyInvitation> ReceivePartyInvitation(std::string senderId, std::string sceneId);
		void RemovePartyInvitation(std::string senderId);
		void AnswerPartyInvitation(std::string senderId, bool accept = true);

		std::list<std::shared_ptr<PartyInvitation>> GetPartyInvitations();

		void SendPartyRequest(std::string userId, pplx::cancellation_token_source cts);
		void ClosePartyRequest(std::string userId);

		std::list<std::string> GetPartyRequestUsersIds();

	private:

		std::unordered_map<std::string, std::shared_ptr<PartyInvitation>> partyInvitations;
		std::unordered_map<std::string, pplx::cancellation_token_source> partyRequests;

	};

	class PartyManagement : public ClientAPI<PartyManagement>
	{
	public:
		PartyManagement(std::weak_ptr<AuthenticationService> auth, std::weak_ptr<ILogger> logger, std::weak_ptr<GameFinder> gameFinder);

		pplx::task<Party_ptr> joinPartySceneByPlatformSessionId(const std::string uniqueOnlinePartyName);
		pplx::task<Party_ptr> joinPartySceneByConnectionToken(const std::string& connectionToken);
		pplx::task<void> leaveParty();
		pplx::task<Party_ptr> getParty();


		pplx::task<void> updatePlayerStatus(PartyUserStatus playerStatus);
		pplx::task<void> updatePartySettings(PartySettingsDto partySettings);
		pplx::task<void> updatePlayerData(std::string data);
		pplx::task<bool> PromoteLeader(std::string userId);
		pplx::task<bool> kickPlayer(std::string userId);
		pplx::task<Party_ptr> createParty(const  PartyRequestDto& partyRequestDto);

		pplx::task<void> sendInvitation(const std::string& id);

		Event<PartySettings> onUpdatedPartySettings;
		Event<std::vector<PartyUserDto>> onUpdatedPartyMembers;
		Event<PartyUserData> onUpdatedUserData;
		Event<void> onJoinedParty;
		Event<void> onKickedFromParty;
		Event<void> onLeftParty;

		PartyInvitations invitations;

		void initialize();
	private:
		

		pplx::task<Party_ptr> getPartySceneByOnlinePartyName(const std::string uniqueOnlinePartyName);
		pplx::task<Party_ptr> getPartySceneByToken(const std::string& connectionToken);

		Party_ptr initPartyFromScene(std::shared_ptr<Scene> scene);
		///
		/// User Exposed callback


		std::weak_ptr<AuthenticationService> _auth;
		std::weak_ptr<ILogger> _logger;
		std::weak_ptr<GameFinder> _gameFinder;
		rxcpp::subscription _partySceneConnectionStateSubscription;

		bool listened;
		std::string _uniqueOnlinePartyName;
		std::shared_ptr<pplx::task<Party_ptr>> _party;

		pplx::task<std::shared_ptr<PartyManagementService>> getPartyManagementService();

		
	};

	class PartyManagementService : public std::enable_shared_from_this<PartyManagementService>
	{

	public:

		PartyManagementService(std::shared_ptr<Scene> scene);

		pplx::task<std::string> createParty(const  PartyRequestDto& partyRequestDto);

		Event<std::string>  PlatformSessionCreated;

	private:
		std::shared_ptr<ILogger> _logger;
		std::weak_ptr<Scene> _scene;

		//Internal callbacks
		std::function<void(RpcRequestContext_ptr requestContext)> _onCreateSession;
	};
}
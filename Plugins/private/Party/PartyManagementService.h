#pragma once

#include "stormancer/ClientAPI.h"
#include "stormancer/Event.h"
#include "Party/PartyModels.h"

namespace Stormancer
{
	class AuthenticationService;
	class ILogger;
	class GameFinder;
	class PartyManagementService;
	
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
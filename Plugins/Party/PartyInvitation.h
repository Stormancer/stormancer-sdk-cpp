#pragma once
//#include "stormancer/stormancer.h"
#include <memory>
#include "GameFinder/GameFinderService.h"

//Forward declares
class Scene;
class ILogger;
class RpcService;

namespace Stormancer
{

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

		Action<bool> onAnswer;

	};

	class PartyInvitationService : public std::enable_shared_from_this<PartyInvitationService>
	{
	public:
		PartyInvitationService();

		///
		/// Sent to server the new party status
		///
		std::shared_ptr<PartyInvitation> ReceivePartyInvitation(std::string senderId, std::string sceneId);
		void RemovePartyInvitation(std::string senderId);
		void AnswerPartyInvitation(std::string senderId, bool accept = true);

		std::list<std::shared_ptr<PartyInvitation>> GetPartyInvitations();

		void SendPartyRequest(std::string userId, rxcpp::composite_subscription observable);
		void CancelPartyRequest(std::string userId);

		std::list<std::string> GetPartyRequestUsersIds();

	private:

		std::unordered_map<std::string, std::shared_ptr<PartyInvitation>> partyInvitations;
		std::unordered_map<std::string, rxcpp::composite_subscription> partyRequests;

	};
}

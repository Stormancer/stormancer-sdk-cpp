#if defined(STORMANCER_CUSTOM_PCH)
#include STORMANCER_CUSTOM_PCH
#endif
#include "PartyManagementService.h"
#include "stormancer/Logger/ILogger.h"
#include "stormancer/RPC/Service.h"
#include "stormancer/exceptions.h"

namespace Stormancer
{
	PartyManagementService::PartyManagementService(std::shared_ptr<Scene> scene)
	{
		_scene = scene;
		_logger = scene->dependencyResolver()->resolve<ILogger>();
	}

	pplx::task<std::string> PartyManagementService::createParty(const PartyRequestDto& partyRequestDto)
	{
		auto rpc = _scene.lock()->dependencyResolver()->resolve<RpcService>();
		return rpc->rpc<std::string, PartyRequestDto>("partymanagement.createsession", partyRequestDto);
	}

	///////////////////////////////
	///Party Invitations
	//////////////////////////////

	std::shared_ptr<PartyInvitation> PartyInvitations::ReceivePartyInvitation(std::string senderId, std::string sceneId)
	{
		auto invitation = std::make_shared<PartyInvitation>(senderId, sceneId);

		if (partyInvitations.find(senderId) == partyInvitations.end())
		{
			partyInvitations[senderId] = invitation;

		}
		else
		{
			throw std::runtime_error(&("Could not receive party invitation as there is already one from " + senderId)[0]);
		}
		return invitation;
	}

	void PartyInvitations::RemovePartyInvitation(std::string senderId)
	{
		if (partyInvitations.find(senderId) != partyInvitations.end())
		{
			partyInvitations.erase(senderId);
		}
		else
		{
			throw std::runtime_error(&("Could not remove pending party invitation as there is none from " + senderId)[0]);
		}
	}

	void PartyInvitations::AnswerPartyInvitation(std::string senderId, bool accept)
	{
		if (partyInvitations.find(senderId) != partyInvitations.end())
		{
			auto invitation = partyInvitations[senderId];
			if (invitation)
			{
				invitation->onAnswer(accept);
				RemovePartyInvitation(senderId);
			}
			else
			{
				throw std::runtime_error(&("Could not answer pending party invitation from " + senderId + " as it is null")[0]);
			}
		}
		else
		{
			throw std::runtime_error(&("Could not answer pending party invitation as there is none from " + senderId)[0]);
		}
	}

	std::list<std::shared_ptr<PartyInvitation>> PartyInvitations::GetPartyInvitations()
	{
		std::list<std::shared_ptr<PartyInvitation>> list;
		for (auto inv : partyInvitations)
		{
			if (inv.second)
			{
				list.push_back(inv.second);
			}
		}
		return list;
	}

	void PartyInvitations::SendPartyRequest(std::string userId, pplx::cancellation_token_source cts)
	{
		if (partyRequests.find(userId) == partyRequests.end())
		{
			partyRequests[userId] = cts;
		}
		else
		{
			throw std::runtime_error(&("Could not send a party request as there is already one for " + userId)[0]);
		}
	}

	void PartyInvitations::ClosePartyRequest(std::string userId)
	{
		if (partyRequests.find(userId) != partyRequests.end())
		{
			partyRequests[userId].cancel();
			partyRequests.erase(userId);
		}
		else
		{
			throw std::runtime_error(&("Could not cancel a party request as there none pending for " + userId)[0]);
		}
	}

	std::list<std::string> PartyInvitations::GetPartyRequestUsersIds()
	{
		std::list<std::string> list;
		for (auto request : partyRequests)
		{
			list.push_back(request.first);
		}
		return list;
	}
}



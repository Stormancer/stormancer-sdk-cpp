#include "Party/PartyInvitation.h"

Stormancer::PartyInvitationService::PartyInvitationService()
{
	partyInvitations.reserve(16);
	partyRequests.reserve(16);
}

std::shared_ptr<Stormancer::PartyInvitation> Stormancer::PartyInvitationService::ReceivePartyInvitation(std::string senderId, std::string sceneId)
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

void Stormancer::PartyInvitationService::RemovePartyInvitation(std::string senderId)
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

void Stormancer::PartyInvitationService::AnswerPartyInvitation(std::string senderId, bool accept)
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
			throw std::runtime_error(&("Could not answer pending party invitation from " + senderId+" as it is null")[0]);
		}
	}
	else
	{
		throw std::runtime_error(&("Could not answer pending party invitation as there is none from " + senderId)[0]);
	}
}

std::list<std::shared_ptr<Stormancer::PartyInvitation>> Stormancer::PartyInvitationService::GetPartyInvitations()
{
	std::list<std::shared_ptr<Stormancer::PartyInvitation>> list;
	for (auto inv : partyInvitations)
	{
		if (inv.second)
		{
			list.push_back(inv.second);
		}
	}
	return list;
}

void Stormancer::PartyInvitationService::SendPartyRequest(std::string userId, rxcpp::composite_subscription observable)
{
	if (partyRequests.find(userId) == partyRequests.end())
	{
		partyRequests[userId] = observable;
	}
	else
	{
		throw std::runtime_error(&("Could not send a party request as there is already one for " + userId)[0]);
	}
}

void Stormancer::PartyInvitationService::CancelPartyRequest(std::string userId)
{
	if (partyRequests.find(userId) != partyRequests.end())
	{
		partyRequests[userId].unsubscribe();
		partyRequests.erase(userId);
	}
	else
	{
		throw std::runtime_error(&("Could not cancel a party request as there none pending for " + userId)[0]);
	}
}

std::list<std::string> Stormancer::PartyInvitationService::GetPartyRequestUsersIds()
{
	std::list<std::string> list;
	for (auto request : partyRequests)
	{
		list.push_back(request.first);
	}
	return list;
}

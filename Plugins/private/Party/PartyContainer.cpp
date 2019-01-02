#if defined(STORMANCER_CUSTOM_PCH)
#include STORMANCER_CUSTOM_PCH
#endif
#include "PartyContainer.h"
#include "stormancer/Scene.h"
#include "Authentication/AuthenticationService.h"

namespace Stormancer
{
	////////////////////////////////////////////////////////////////////////////////////
	/// Party
	////////////////////////////////////////////////////////////////////////////////////
	PartyContainer::PartyContainer(std::shared_ptr<Scene> scene,
		Event<void>::Subscription JoinedPartySubscription,
		Event<void>::Subscription LeftPartySubscription,
		Event<void>::Subscription KickedFromPartySubscription,
		Event<std::vector<PartyUserDto>>::Subscription UpdatedPartyMembersSubscription,
		Event<PartyUserData>::Subscription UpdatedPartyUserDataSubscription,
		Event<PartySettings>::Subscription UpdatedPartySettingsSubscription) 
		:_partyScene(scene),
		LeftPartySubscription(LeftPartySubscription),
		UpdatedPartyMembersSubscription(UpdatedPartyMembersSubscription),
		UpdatedPartyUserDataSubscription(UpdatedPartyUserDataSubscription),
		UpdatedPartySettingsSubscription(UpdatedPartySettingsSubscription),
		JoinedPartySubscription(JoinedPartySubscription),
		KickedFromPartySubscription(KickedFromPartySubscription)
	{

	}

	std::shared_ptr<Scene> PartyContainer::getScene()
	{
		return _partyScene;
	}

	bool PartyContainer::isLeader()
	{
		return settings().leaderId == _partyScene->dependencyResolver()->resolve<AuthenticationService>()->userId();
	}
}
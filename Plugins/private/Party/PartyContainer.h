#pragma once
#include "stormancer/Event.h"
#include "Party/PartyModels.h"
#include "PartyService.h"
#include "stormancer/Scene.h"

namespace Stormancer
{
	class PartyContainer
	{
	public:
		PartyContainer(
			std::shared_ptr<Scene> scene,
			Event<void>::Subscription JoinedPartySubscription,
			Event<void>::Subscription LeftPartySubscription,
			Event<void>::Subscription KickedFromPartySubscription,
			Event<std::vector<PartyUserDto>>::Subscription UpdatedPartyMembersSubscription,
			Event<PartyUserData>::Subscription UpdatedPartyUserDataSubscription,
			Event<PartySettings>::Subscription UpdatedPartySettingsSubscription
		);

		bool is_settings_valid() const
		{
			return (_partyScene != nullptr && _partyScene->dependencyResolver()->resolve<PartyService>());
		}

		PartySettings& settings()
		{
			return  _partyScene->dependencyResolver()->resolve<PartyService>()->settings();
		}

		std::vector<PartyUserDto>& members()
		{
			return _partyScene->dependencyResolver()->resolve<PartyService>()->members();
		}

		bool isLeader();
		std::shared_ptr<Scene> getScene();
		std::string id()
		{
			return _partyScene->id();
		}

	private:
		std::shared_ptr<Scene> _partyScene;

		Event<void>::Subscription LeftPartySubscription;
		Event<void>::Subscription JoinedPartySubscription;
		Event<void>::Subscription KickedFromPartySubscription;
		Event<std::vector<PartyUserDto>>::Subscription UpdatedPartyMembersSubscription;
		Event<PartyUserData>::Subscription UpdatedPartyUserDataSubscription;
		Event<PartySettings>::Subscription UpdatedPartySettingsSubscription;
	};
}

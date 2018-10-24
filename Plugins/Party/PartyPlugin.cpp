#include "stormancer/Scene.h"
#include "Party/PartyPlugin.h"
#include "Party/PartyInvitation.h"
#include "Party/PartyService.h"
#include "Party/PartyManagement.h"
#include "Authentication/AuthenticationService.h"
#include "GameFinder/GameFinderManager.h"

namespace Stormancer
{
	void PartyPlugin::sceneCreated(Scene* scene)
	{
		if (scene)
		{
			auto name = scene->getHostMetadata("stormancer.party");
			if (name.length() > 0)
			{
				auto service = std::make_shared<PartyService>(scene->shared_from_this());
				scene->dependencyResolver().lock()->registerDependency<PartyService>(service);
			}

			name = scene->getHostMetadata("stormancer.partymanagement");
			if (name.length() > 0)
			{
				auto service = std::make_shared<PartyManagementService>(scene->shared_from_this());
				scene->dependencyResolver().lock()->registerDependency<PartyManagementService>(service);
			}
		}
	}

	void PartyPlugin::sceneDisconnected(Scene* scene)
	{
		if (scene)
		{
			auto name = scene->getHostMetadata("stormancer.party");
			if (name.length() > 0)
			{
				auto partyService = scene->dependencyResolver().lock()->resolve<PartyService>();
				partyService->onDisconnected();
			}
		}
	}

	void PartyPlugin::clientCreated(Client* client)
	{
		if (client)
		{
			auto service = std::make_shared<PartyInvitationService>();
			client->dependencyResolver().lock()->registerDependency<PartyInvitationService>(service);
		}
		if (client)
		{
			client->dependencyResolver().lock()->registerDependency<PartyManagement>([](std::weak_ptr<DependencyResolver> dr) {
				return std::make_shared<PartyManagement>(dr.lock()->resolve<AuthenticationService>(), dr.lock()->resolve<ILogger>(), dr.lock()->resolve<GameFinder>()); }, true);
		}
	}

};

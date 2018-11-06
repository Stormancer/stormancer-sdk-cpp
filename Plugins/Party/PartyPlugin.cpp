#include "stormancer/Scene.h"
#include "Party/PartyPlugin.h"
#include "Party/PartyInvitation.h"
#include "Party/PartyService.h"
#include "Party/PartyManagement.h"
#include "Authentication/AuthenticationService.h"
#include "GameFinder/GameFinderManager.h"

namespace Stormancer
{
	void PartyPlugin::sceneCreated(std::shared_ptr<Scene> scene)
	{
		if (scene)
		{
			auto name = scene->getHostMetadata("stormancer.party");
			if (name.length() > 0)
			{
				auto service = std::make_shared<PartyService>(scene);
				scene->dependencyResolver()->registerDependency<PartyService>(service);
			}

			name = scene->getHostMetadata("stormancer.partymanagement");
			if (name.length() > 0)
			{
				auto service = std::make_shared<PartyManagementService>(scene);
				scene->dependencyResolver()->registerDependency<PartyManagementService>(service);
			}
		}
	}

	void PartyPlugin::sceneDisconnected(std::shared_ptr<Scene> scene)
	{
		if (scene)
		{
			auto name = scene->getHostMetadata("stormancer.party");
			if (name.length() > 0)
			{
				auto partyService = scene->dependencyResolver()->resolve<PartyService>();
				partyService->onDisconnected();
			}
		}
	}

	void PartyPlugin::clientCreated(std::shared_ptr<IClient> client)
	{
		if (client)
		{
			auto service = std::make_shared<PartyInvitationService>();
			client->dependencyResolver()->registerDependency<PartyInvitationService>(service);
		}
		if (client)
		{
			client->dependencyResolver()->registerDependency<PartyManagement>([](std::weak_ptr<DependencyResolver> dr) {
				return std::make_shared<PartyManagement>(dr.lock()->resolve<AuthenticationService>(), dr.lock()->resolve<ILogger>(), dr.lock()->resolve<GameFinder>()); }, true);
		}
	}

};

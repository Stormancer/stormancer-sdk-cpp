#if defined(STORMANCER_CUSTOM_PCH)
#include STORMANCER_CUSTOM_PCH
#endif
#include "PartyPlugin.h"
#include "stormancer/Scene.h"
#include "PartyService.h"
#include "Party_Impl.h"
#include "Party/Party.h"
#include "Authentication/AuthenticationService.h"
#include "GameFinder/GameFinder.h"

namespace Stormancer
{
	void PartyPlugin::registerSceneDependencies(ContainerBuilder& builder, std::shared_ptr<Scene> scene)
	{
		if (scene)
		{
			auto name = scene->getHostMetadata("stormancer.party");
			if (name.length() > 0)
			{
				builder.registerDependency<PartyService, Scene>().singleInstance();
			}

			name = scene->getHostMetadata("stormancer.partymanagement");
			if (name.length() > 0)
			{
				builder.registerDependency<PartyManagementService, Scene>().singleInstance();
			}
		}
	}

	void PartyPlugin::sceneCreated(std::shared_ptr<Scene> scene)
	{
		scene->dependencyResolver().resolve<PartyService>()->initialize();
	}

	void PartyPlugin::registerClientDependencies(ContainerBuilder& builder)
	{
		builder.registerDependency<Party>([](const DependencyScope& dr) {
			auto service =  std::make_shared<Party_Impl>(dr.resolve<AuthenticationService>(), dr.resolve<ILogger>(), dr.resolve<GameFinder>());
			service->initialize();
			return service;
		}).singleInstance();
	}

};

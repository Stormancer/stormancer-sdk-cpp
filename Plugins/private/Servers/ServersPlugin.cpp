#if defined(STORMANCER_CUSTOM_PCH)
#include STORMANCER_CUSTOM_PCH
#endif
#include "ServersPlugin.h"
#include "ServersService.h"
#include "Servers_Impl.h"
#include "Servers/Servers.h"
#include "stormancer/IClient.h"
#include "stormancer/DependencyInjection.h"

namespace Stormancer
{
	void registerClientDependencies(ContainerBuilder& builder)
	{
		builder.registerDependency<Servers_Impl, AuthenticationService>().as<Servers>().singleInstance();
	}

	void ServerPlugin::registerSceneDependencies(ContainerBuilder& builder, std::shared_ptr<Scene> scene)
	{
		if (scene)
		{
			auto name = scene->getHostMetadata("stormancer.leaderboard");

			if (!name.empty())
			{
				builder.registerDependency<ServersService, Scene>().singleInstance();
			}
		}
	}
}

#include "Helloworld/HelloworldPlugin.h"
#include "stormancer/Scene.h"
#include "Helloworld/Hello.h"
#include "Helloworld/HelloService.h"

void Helloworld::HelloworldPlugin::registerSceneDependencies(Stormancer::ContainerBuilder& builder, std::shared_ptr<Stormancer::Scene> scene)
{
	if (scene)
	{
		auto name = scene->getHostMetadata("helloworld");

		if (!name.empty())
		{
			builder.registerDependency<Helloworld::HelloService, Stormancer::Scene>().singleInstance();
		}
	}
}

void Helloworld::HelloworldPlugin::registerClientDependencies(Stormancer::ContainerBuilder& builder)
{
	builder.registerDependency<Helloworld::Hello, Stormancer::Users::UsersApi>().singleInstance();
}

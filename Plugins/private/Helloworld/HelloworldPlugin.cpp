#include "Helloworld/HelloworldPlugin.h"
#include "stormancer/Scene.h"
#include "Helloworld/Hello.h"
#include "Helloworld/HelloService.h"

void Helloworld::HelloworldPlugin::sceneCreated(std::shared_ptr<Stormancer::Scene> scene)
{
	if (scene)
	{
		auto name = scene->getHostMetadata("helloworld");

		if (!name.empty())
		{
			auto service = std::make_shared<Helloworld::HelloService>(scene);
			scene->dependencyResolver()->registerDependency<Helloworld::HelloService>(service);
		}
	}
}

void Helloworld::HelloworldPlugin::clientCreated(std::shared_ptr<Stormancer::IClient> client)
{
	if (client)
	{
		client->dependencyResolver()->registerDependency<Helloworld::Hello>([](std::weak_ptr<Stormancer::DependencyResolver> dr) {
			return std::make_shared<Helloworld::Hello>(dr.lock()->resolve<Stormancer::AuthenticationService>()); }, true);
	}
}

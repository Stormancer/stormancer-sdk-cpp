#include "stormancer/headers.h"
#include "TeamsPlugin.h"
#include "TeamsService.h"
#include "stormancer/Scene.h"

namespace Stormancer
{
	void TeamsPlugin::sceneCreated(Scene* scene)
	{
		if (scene)
		{
			auto name = scene->getHostMetadata("stormancer.teams");

			if (!name.empty())
			{
				auto service = std::make_shared<TeamsService>(scene->shared_from_this());
				scene->dependencyResolver().lock()->registerDependency<TeamsService>(service);
			}
		}
	}
}

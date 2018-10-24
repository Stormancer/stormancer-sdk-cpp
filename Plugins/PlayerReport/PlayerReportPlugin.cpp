#include "Stormancer/stdafx.h"
#include "stormancer/headers.h"
#include "PlayerReportPlugin.h"
#include "PlayerReportService.h"

namespace Stormancer
{
	void PlayerReportPlugin::sceneCreated(Scene* scene)
	{
		if (scene)
		{
			auto name = scene->getHostMetadata("stormancer.bugReporting");
			if (name.length() > 0)
			{
				auto service = std::make_shared<PlayerReportService>(scene->shared_from_this());
				scene->dependencyResolver().lock()->registerDependency<PlayerReportService>(service);
			}
		}
	}
}

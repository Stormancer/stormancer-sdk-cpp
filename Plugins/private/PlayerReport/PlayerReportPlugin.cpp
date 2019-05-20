#if defined(STORMANCER_CUSTOM_PCH)
#include STORMANCER_CUSTOM_PCH
#endif
#include "PlayerReport/PlayerReportPlugin.h"

#include "PlayerReport/PlayerReportService.h"

namespace Stormancer
{
	void PlayerReportPlugin::sceneCreated(std::shared_ptr<Scene> scene)
	{
		if (scene)
		{
			auto name = scene->getHostMetadata("stormancer.bugReporting");
			if (name.length() > 0)
			{
				auto service = std::make_shared<PlayerReportService>(scene);
				scene->dependencyResolver()->registerDependency<PlayerReportService>(service);
			}
		}
	}
}

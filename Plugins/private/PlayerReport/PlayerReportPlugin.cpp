#if defined(STORMANCER_CUSTOM_PCH)
#include STORMANCER_CUSTOM_PCH
#endif
#include "PlayerReport/PlayerReportPlugin.h"

#include "PlayerReport/PlayerReportService.h"

namespace Stormancer
{
	void PlayerReportPlugin::registerSceneDependencies(ContainerBuilder& builder, std::shared_ptr<Scene> scene)
	{
		if (scene)
		{
			auto name = scene->getHostMetadata("stormancer.bugReporting");
			if (name.length() > 0)
			{
				builder.registerDependency<PlayerReportService, Scene>().singleInstance();
			}
		}
	}
}

#include "BugReport/BugReportPlugin.h"
#include "BugReport/BugReportService.h"

namespace Stormancer
{
	void BugReportPlugin::registerSceneDependencies(ContainerBuilder& builder, std::shared_ptr<Scene> scene)
	{
		if (scene)
		{
			auto name = scene->getHostMetadata("stormancer.bugReporting");
			if (name.length() > 0)
			{
				builder.registerDependency<BugReportService, Scene>();
			}
		}
	}
}

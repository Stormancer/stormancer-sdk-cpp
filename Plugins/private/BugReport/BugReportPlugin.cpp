#include "BugReport/BugReportPlugin.h"
#include "BugReport/BugReportService.h"

namespace Stormancer
{
	void BugReportPlugin::sceneCreated(std::shared_ptr<Scene> scene)
	{
		if (scene)
		{
			auto name = scene->getHostMetadata("stormancer.bugReporting");
			if (name.length() > 0)
			{
				auto service = std::make_shared<BugReportService>(scene);
				scene->dependencyResolver()->registerDependency<BugReportService>(service);
			}
		}
	}
}

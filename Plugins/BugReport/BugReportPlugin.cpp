#include "stdafx.h"
#include "BugReportPlugin.h"
#include "BugReportService.h"

namespace Stormancer
{
	void BugReportPlugin::sceneCreated(Scene* scene)
	{
		if (scene)
		{
			auto name = scene->getHostMetadata("stormancer.bugReporting");
			if (name.length() > 0)
			{
				auto service = std::make_shared<BugReportService>(scene->shared_from_this());
				scene->dependencyResolver()->registerDependency<BugReportService>(service);
			}
		}
	}
};

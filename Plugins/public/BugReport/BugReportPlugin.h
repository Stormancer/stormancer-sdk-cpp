#pragma once

#include "stormancer/IPlugin.h"
#include <memory>

namespace Stormancer
{
	class BugReportPlugin : public IPlugin
	{
	public:
		void sceneCreated(std::shared_ptr<Scene> scene) override;
	};
};

#pragma once

#include "stormancer/IPlugin.h"

namespace Stormancer
{
	class PlayerReportPlugin : public IPlugin
	{
	public:
		void sceneCreated(std::shared_ptr<Scene> scene) override;
	};
};

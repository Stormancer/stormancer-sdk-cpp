#pragma once

#include "cpprest/json.h"
#include "Stormancer/IPlugin.h"
#include "Stormancer/Scene.h"

namespace Stormancer
{
	class InAppNotificationPlugin : public IPlugin
	{
	public:

		void sceneCreated(std::shared_ptr<Scene> scene) override;
	};
}

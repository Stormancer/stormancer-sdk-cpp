#pragma once
#include "stormancer/IPlugin.h"
#include "stormancer/Scene.h"

namespace Stormancer
{
	class InAppNotificationPlugin : public IPlugin
	{
	public:

		void sceneCreated(std::shared_ptr<Scene> scene) override;
	};
}

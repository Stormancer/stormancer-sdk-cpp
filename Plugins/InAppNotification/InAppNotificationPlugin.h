#pragma once
#include "Stormancer/headers.h"
#include "cpprest/json.h"
#include "Stormancer/IPlugin.h"
#include "Stormancer/Scene.h"

namespace Stormancer
{
	class InAppNotificationPlugin : public IPlugin
	{
	public:

		void sceneCreated(Scene* scene) override;
	};
}
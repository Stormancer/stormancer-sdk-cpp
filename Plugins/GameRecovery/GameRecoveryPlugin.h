#pragma once
#include "stormancer/headers.h"
#include "stormancer/IPlugin.h"
#include "stormancer/Scene.h"
#include <cpprest/json.h>

namespace Stormancer
{
	class GameRecoveryPlugin : public IPlugin
	{
	public:

		void sceneCreated(Scene* scene) override;
		void clientCreated(Client* client) override;
	};
}

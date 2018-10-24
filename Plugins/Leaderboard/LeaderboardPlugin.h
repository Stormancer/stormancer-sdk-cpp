#pragma once

#include <cpprest/json.h>
#include "stormancer/headers.h"
#include "stormancer/IPlugin.h"
#include "stormancer/Scene.h"

namespace Stormancer
{
	class LeaderboardPlugin : public IPlugin
	{
	public:

		void sceneCreated(Scene* scene) override;
		void clientCreated(Client* client) override;
	};
}

#pragma once
#include "stormancer/headers.h"
#include "stormancer/IPlugin.h"

namespace Stormancer
{
	class GameFinderPlugin : public IPlugin
	{
	public:
		void sceneCreated(Scene* scene) override;

		void clientCreated(Client* client) override;

	};
};

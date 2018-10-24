#pragma once
#include "stormancer/IPlugin.h"

class Scene;

namespace Stormancer
{
	class PartyPlugin : public IPlugin
	{	
	public:
		void sceneCreated(Scene* scene) override;
		void sceneDisconnected(Scene* scene) override;
		void clientCreated(Client* client) override;
	};
};

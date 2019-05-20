#pragma once

#include "stormancer/IPlugin.h"

class Scene;

namespace Stormancer
{
	class PartyPlugin : public IPlugin
	{	
	public:
		void sceneCreated(std::shared_ptr<Scene> scene) override;
		void clientCreated(std::shared_ptr<IClient> client) override;
	};
};

#pragma once

#include "stormancer/IPlugin.h"

namespace Stormancer
{
	class IClient; 
	class Scene;

	class FriendsPlugin: public IPlugin
	{
	public:
		void sceneCreated(std::shared_ptr<Scene> scene) override;

		void clientCreated(std::shared_ptr<IClient> client) override;
	};
}

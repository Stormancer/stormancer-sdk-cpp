#pragma once
#include "stormancer/headers.h"
#include "stormancer/IPlugin.h"

namespace Stormancer
{
	class ProfilePlugin : public IPlugin
	{
	public:
		ProfilePlugin();
		~ProfilePlugin();
		void sceneCreated(std::shared_ptr<Scene> scene) override;
		void clientCreated(std::shared_ptr<IClient> client) override;
	};
};

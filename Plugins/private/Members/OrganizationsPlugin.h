#pragma once
#include "stormancer/headers.h"
#include "stormancer/IPlugin.h"

namespace Stormancer
{
	class OrganizationsPlugin : public IPlugin
	{
	public:

		void clientCreated(std::shared_ptr<IClient> client) override;

		void sceneCreated(std::shared_ptr<Scene> scene) override;
	};
};

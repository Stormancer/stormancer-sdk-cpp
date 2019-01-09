#pragma once

#include "stormancer/headers.h"
#include "stormancer/IPlugin.h"
#include "stormancer/IClient.h"

namespace Stormancer
{
	class AuthenticationPlugin : public IPlugin
	{
	public:
		AuthenticationPlugin();
		~AuthenticationPlugin();
		void clientCreated(std::shared_ptr<IClient> client) override;
		void clientDisconnecting(std::shared_ptr<IClient> client) override;
	};
};

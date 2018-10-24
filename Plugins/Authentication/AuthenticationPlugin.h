#pragma once

#include "stormancer/headers.h"
#include "stormancer/IPlugin.h"
#include "stormancer/Client.h"

namespace Stormancer
{
	class AuthenticationPlugin : public IPlugin
	{
	public:
		AuthenticationPlugin();
		~AuthenticationPlugin();
		void clientCreated(Client* client) override;
		void clientDisconnecting(Client* client) override;
	};
};

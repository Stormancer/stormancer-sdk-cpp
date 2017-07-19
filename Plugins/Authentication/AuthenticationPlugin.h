#pragma once
#include <headers.h>
#include <IPlugin.h>
#include <Client.h>

namespace Stormancer
{
	class AuthenticationPlugin : public IPlugin
	{
	public:
		AuthenticationPlugin();
		~AuthenticationPlugin();
		void clientCreated(Client* client) override;
	};
};

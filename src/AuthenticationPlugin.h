#pragma once
#include "headers.h"

namespace Stormancer
{
	class AuthenticationPlugin : public IClientPlugin
	{
	public:
		AuthenticationPlugin();
		virtual ~AuthenticationPlugin();

		void build(PluginBuildContext* ctx);
		
	private:
		void registerAuthenticationService(Client* client);
	};
};

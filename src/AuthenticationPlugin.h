#pragma once
#include "headers.h"
#include "IPlugin.h"

namespace Stormancer
{
	class AuthenticationPlugin : public IPlugin
	{
	public:
		AuthenticationPlugin();
		virtual ~AuthenticationPlugin();

		void clientCreated(Client* client);

		void destroy();
	};
};

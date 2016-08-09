#pragma once
#include <stormancer.h>

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

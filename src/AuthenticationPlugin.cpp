#include "stormancer.h"

namespace Stormancer
{
	AuthenticationPlugin::AuthenticationPlugin()
	{
	}

	AuthenticationPlugin::~AuthenticationPlugin()
	{
	}

	void AuthenticationPlugin::clientCreated(Client* client)
	{
		auto authService = new AuthenticationService(client);
		client->dependencyResolver()->registerDependency<IAuthenticationService>(authService);
	}

	void AuthenticationPlugin::destroy()
	{
		delete this;
	}
};

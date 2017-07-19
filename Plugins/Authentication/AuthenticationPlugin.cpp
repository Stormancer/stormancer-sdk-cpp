#include <stdafx.h>
#include "AuthenticationPlugin.h"
#include "AuthenticationService.h"

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
		auto authService = std::make_shared<AuthenticationService>(client);
		client->dependencyResolver()->registerDependency<AuthenticationService>(authService);
	}
};

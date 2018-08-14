#include "stormancer/headers.h"
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
		client->dependencyResolver().lock()->registerDependency<AuthenticationService>(authService);
	}
};
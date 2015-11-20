#include "stormancer.h"

namespace Stormancer
{
	AuthenticationPlugin::AuthenticationPlugin()
	{
	}

	AuthenticationPlugin::~AuthenticationPlugin()
	{
	}

	void AuthenticationPlugin::build(PluginBuildContext* ctx)
	{
		ctx->clientCreated += std::bind(&AuthenticationPlugin::registerAuthenticationService, this, std::placeholders::_1);
	}

	void AuthenticationPlugin::registerAuthenticationService(Client* client)
	{
		auto authService = new AuthenticationService(client);
		client->dependencyResolver()->registerDependency<IAuthenticationService>(authService);
	}
};

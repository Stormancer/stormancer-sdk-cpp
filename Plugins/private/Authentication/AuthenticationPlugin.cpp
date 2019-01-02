#if defined(STORMANCER_CUSTOM_PCH)
#include STORMANCER_CUSTOM_PCH
#endif
#include "Authentication/AuthenticationPlugin.h"
#include "Authentication/AuthenticationService.h"

namespace Stormancer
{
	AuthenticationPlugin::AuthenticationPlugin()
	{
	}

	AuthenticationPlugin::~AuthenticationPlugin()
	{
	}

	void AuthenticationPlugin::clientCreated(std::shared_ptr<IClient> client)
	{
		auto authService = std::make_shared<AuthenticationService>(client);
		client->dependencyResolver()->registerDependency<AuthenticationService>(authService);
		
	}

	void AuthenticationPlugin::clientDisconnecting(std::shared_ptr<IClient> client)
	{
		auto auth = client->dependencyResolver()->resolve<AuthenticationService>();
		auth->logout();
	}
};
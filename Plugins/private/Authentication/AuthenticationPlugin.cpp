#include "Authentication/AuthenticationPlugin.h"
#include "Authentication/AuthenticationService.h"

namespace Stormancer
{
	void AuthenticationPlugin::registerClientDependencies(ContainerBuilder& builder)
	{
		builder.registerDependency<AuthenticationService, IClient>().singleInstance();
	}

	void AuthenticationPlugin::clientDisconnecting(std::shared_ptr<IClient> client)
	{
		auto auth = client->dependencyResolver().resolve<AuthenticationService>();
		auth->logout();
	}
};
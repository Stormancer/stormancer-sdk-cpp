#include "ServersService.h"
#include "Servers_Impl.h"
#include "Authentication/AuthenticationService.h"

namespace Stormancer
{

	Servers_Impl::Servers_Impl(std::weak_ptr<AuthenticationService> auth) : ClientAPI(auth)
	{

	}

	pplx::task<std::shared_ptr<ServersService>> Servers_Impl::getServersService()
	{
		return this->getService<ServersService>("stormancer.server");
	}

	pplx::task<std::vector<Stormancer::ServerDescription>> Servers_Impl::GetServersDescription()
	{
		return getServersService().then([](std::shared_ptr<ServersService> service) {return service->Get(); });
	}

	pplx::task<void> Servers_Impl::SelectServer(const std::string serverId)
	{
		return getServersService().then([serverId](std::shared_ptr<ServersService> service) {return service->Select(serverId); });
	}
}

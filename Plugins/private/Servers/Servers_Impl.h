#pragma once
#include "Core/ClientAPI.h"
#include "Servers/Servers.h"
#include "Servers/ServersModels.h"

namespace Stormancer
{
	class ServersService;

	class Servers_Impl: public ClientAPI<Servers_Impl>, public Servers
	{
	public:
		Servers_Impl(std::weak_ptr<AuthenticationService> auth);

		virtual pplx::task<std::vector<ServerDescription>> GetServersDescription() override;
		
		virtual pplx::task<void> SelectServer(const std::string serveurId) override;
	private:
		pplx::task<std::shared_ptr<ServersService>> getServersService();
	};
}
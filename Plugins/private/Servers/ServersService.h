#pragma once
#include "stormancer/RPC/Service.h"
#include "Servers/ServersModels.h"

namespace Stormancer
{
	class ServersService
	{
	public:
		ServersService(std::weak_ptr<Scene> scene);

		pplx::task<std::vector<ServerDescription>> Get();
		
		pplx::task<void> Select(const std::string serverId);

	private:

		std::weak_ptr<Scene> _scene;
		std::shared_ptr<RpcService> _rpcService;

	};
}

#include "ServersService.h"

namespace Stormancer
{
	ServersService::ServersService(std::weak_ptr<Scene> scene)
		: _scene(scene),
		_rpcService(_scene.lock()->dependencyResolver().resolve<RpcService>())
	{		
	}

	pplx::task<std::vector<ServerDescription>> ServersService::Get()
	{
		return _rpcService->rpc<std::vector<ServerDescription>, int>("Server.Get", 1);
	}

	pplx::task<void> ServersService::Select(const std::string serverId)
	{
		return _rpcService->rpc<void, std::string>("Server.Select", serverId);
	}
}

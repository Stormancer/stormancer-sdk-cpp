#include "HelloService.h"
#include "stormancer/Scene.h"
#include "stormancer/RPC/Service.h"
#include "stormancer/Exceptions.h"

Helloworld::HelloService::HelloService(std::shared_ptr<Stormancer::Scene> scene)
	: _rpcService(scene->dependencyResolver().resolve<Stormancer::RpcService>())
{
}

pplx::task<std::string> Helloworld::HelloService::world(std::string name)
{
	auto rpc = _rpcService.lock();
	if (!rpc)
	{
		throw Stormancer::PointerDeletedException("Scene destroyed");
	}
	return rpc->rpc<std::string>("Hello.World", name);
}

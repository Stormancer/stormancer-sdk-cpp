#pragma once
#include "stormancer/Tasks.h"
namespace Stormancer
{
	class Scene;
	class RpcService;
}
namespace Helloworld
{

	class HelloService
	{
	public:
		HelloService(std::shared_ptr<Stormancer::Scene> scene);

		pplx::task<std::string> world(std::string name);

	private:
		std::weak_ptr<Stormancer::RpcService> _rpcService;

	};
}
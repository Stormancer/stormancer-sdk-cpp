#pragma once
#include "Core/ClientAPI.h"
#include "Authentication/AuthenticationService.h"
#include "stormancer/IPlugin.h"

namespace Stormancer
{
	namespace Helloworld
	{
		namespace details
		{
			class HelloService
			{
			public:
				HelloService(std::shared_ptr<Stormancer::Scene> scene) : _rpcService(scene->dependencyResolver().resolve<Stormancer::RpcService>())
				{
				}

				pplx::task<std::string> world(std::string name)
				{
					auto rpc = _rpcService.lock();
					if (!rpc)
					{
						throw Stormancer::PointerDeletedException("Scene destroyed");
					}
					return rpc->rpc<std::string>("Hello.World", name);
				}

			private:
				std::weak_ptr<Stormancer::RpcService> _rpcService;

			};
		}

		class Hello : public Stormancer::ClientAPI<Hello>
		{
		public:
			Hello(std::weak_ptr<Stormancer::AuthenticationService> auth) : Stormancer::ClientAPI<Hello>(auth) {}
			pplx::task<std::string> world(std::string name)
			{
				return this->getService<details::HelloService>("helloworld")
					.then([name](std::shared_ptr<details::HelloService> hello)
				{
					return hello->world(name);
				});
			}

		};

		class HelloworldPlugin : public Stormancer::IPlugin
		{
		public:

			void registerSceneDependencies(Stormancer::ContainerBuilder& builder, std::shared_ptr<Stormancer::Scene> scene) override
			{
				if (scene)
				{
					auto name = scene->getHostMetadata("helloworld");

					if (!name.empty())
					{
						builder.registerDependency<details::HelloService, Stormancer::Scene>().singleInstance();
					}
				}
			}
			void registerClientDependencies(Stormancer::ContainerBuilder& builder) override
			{
				builder.registerDependency<Hello, Stormancer::AuthenticationService>().singleInstance();
			}
		};
		
	}

}
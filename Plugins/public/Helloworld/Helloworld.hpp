#pragma once
#include "Core/ClientAPI.h"
#include "Authentication/AuthenticationService.h"
#include "stormancer/IPlugin.h"

namespace Stormancer
{
	namespace Helloworld
	{
		class HelloworldPlugin;
		namespace details
		{

			class HelloService :public std::enable_shared_from_this<HelloService>
			{
				friend class HelloworldPlugin;
			public:
				HelloService(std::shared_ptr<Stormancer::RpcService> rpc) : _rpcService(rpc)
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



				//Event fired whenever the service client receives a server message on the Hello.Back route.
				Stormancer::Event<std::string> helloBackReceived;
			private:
				std::weak_ptr<Stormancer::RpcService> _rpcService;

				//Initializes the service
				void initialize(std::shared_ptr<Stormancer::Scene> scene)
				{
					//Capture a weak pointer of this in the route handler to make sure that:
					//* We don't prevent this from being destroyed (capturing a shared pointer)
					//* If destroyed, we don't try to use it in the handler (capturing a this reference directly)
					std::weak_ptr<HelloService> wService = this->shared_from_this();
					scene->addRoute("Hello.Back", [wService](Stormancer::Packetisp_ptr packet) {
						//We expect the message to contain a string
						auto message = packet->readObject<std::string>();
						auto service = wService.lock();
						//If service is valid, forward the event.
						if (service)
						{
							service->helloBackReceived(message);
						}

					});
				}

			};
		}

		class Hello : public Stormancer::ClientAPI<Hello>
		{
			friend class HelloworldPlugin;
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
			Stormancer::Event<std::string> helloBackReceived;


		private:
			void onConnecting(std::shared_ptr <details::HelloService> service)
			{
				std::weak_ptr<Hello> wThis = this->shared_from_this();
				//Always capture weak references, and NEVER 'this'. As the callback is going to be executed asynchronously,
				//who knows what may have happened to the object behind the this pointer since it was captured?
				helloBackReceivedSubscription = service->helloBackReceived.subscribe([wThis](std::string message) {
					auto that = wThis.lock();
					//If this is valid, forward the event.
					if (that)
					{
						that->helloBackReceived(message);
					}
				
				});
			}
			void onDisconnecting(std::shared_ptr <details::HelloService> service)
			{
				//Unsubscribe by destroying the subscription
				helloBackReceivedSubscription = nullptr;
			}

			Stormancer::Event<std::string>::Subscription helloBackReceivedSubscription;
		};

		class HelloworldPlugin : public Stormancer::IPlugin
		{
		public:

			void registerSceneDependencies(Stormancer::ContainerBuilder& builder, std::shared_ptr<Stormancer::Scene> scene) override
			{

				auto name = scene->getHostMetadata("helloworld");

				if (!name.empty())
				{
					builder.registerDependency<details::HelloService>([](const Stormancer::DependencyScope& scope) {
						auto instance = std::make_shared<details::HelloService>(scope.resolve<Stormancer::RpcService>());
						instance->initialize(scope.resolve<Stormancer::Scene>());
						return instance;
					}).singleInstance();
				}

			}
			void registerClientDependencies(Stormancer::ContainerBuilder& builder) override
			{
				builder.registerDependency<Hello, Stormancer::AuthenticationService>().singleInstance();
			}
			void sceneConnecting(std::shared_ptr<Scene> scene) override
			{
				auto name = scene->getHostMetadata("helloworld");

				if (!name.empty())
				{
					auto hello = scene->dependencyResolver().resolve<Hello>();
					auto service = scene->dependencyResolver().resolve<details::HelloService>();
					hello->onConnecting(service);
				}
			}

			void sceneDisconnecting(std::shared_ptr<Scene> scene) override
			{
				auto name = scene->getHostMetadata("helloworld");

				if (!name.empty())
				{
					auto hello = scene->dependencyResolver().resolve<Hello>();
					auto service = scene->dependencyResolver().resolve<details::HelloService>();
					hello->onDisconnecting(service);
				}
			}
		};

	}

}
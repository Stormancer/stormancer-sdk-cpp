#pragma once
#include "Users/ClientAPI.hpp"
#include "Users/Users.hpp"
#include "stormancer/IPlugin.h"

namespace Stormancer
{
	namespace ServerPools
	{
		enum class Status
		{
			Unknown,
			//Server initializing
			Initializing,
			//Server ready to accept a game
			Ready,
			//Game in progress
			InProgress,
			//Game complete
			Complete
		};
		struct Group
		{
			std::string groupId;
			std::vector<std::string> playerIds;

			MSGPACK_DEFINE(groupId, playerIds)
		};
		struct Team
		{
			std::string teamId;
			std::vector<Group> groups;

			MSGPACK_DEFINE(teamId, groups)
		};
		template<typename T>
		struct GameSessionConfiguration
		{
			bool isPublic;
			bool canRestart;
			std::string hostUserId;
			std::vector<Team> teams;
			//parameters is transmitted as a msgpack map. It must therefore use MSGPACK_DEFINE_MAP instead of MSGPACK_DEFINE
			T parameters;
			std::string pool;

			MSGPACK_DEFINE(isPublic, canRestart, hostUserId, teams, parameters, pool)
		};

		template<typename T>
		struct GameSessionStartupParameters
		{
			std::string gameSessionConnectionToken;
			GameSessionConfiguration<T> config;

			MSGPACK_DEFINE(gameSessionConnectionToken, config)
		};

		class ServerPoolsPlugin;

		namespace details
		{
			class ServerPoolsService :public std::enable_shared_from_this<ServerPoolsService>
			{
				friend class ::Stormancer::ServerPools::ServerPoolsPlugin;

			public:
				ServerPoolsService(std::shared_ptr<Stormancer::RpcService> rpc)
					: _rpcService(rpc)

				{
				}

				template<typename T>
				pplx::task<GameSessionStartupParameters<T>> setReady()
				{
					auto rpc = _rpcService.lock();
					if (!rpc)
					{
						throw Stormancer::PointerDeletedException("Scene destroyed");
					}
					return rpc->rpc<GameSessionStartupParameters<T>>("ServerPool.SetReady");
				}

				//Event fired when Stormancer requests a status update from the server
				std::function<Stormancer::ServerPools::Status()> getStatusCallback;

				//Event fired when the service client receives a shutdown request
				Stormancer::Event<void> shutdownReceived;

			private:

				std::weak_ptr<Stormancer::RpcService> _rpcService;

				//Initializes the service
				void initialize(std::shared_ptr<Stormancer::Scene> scene)
				{
					//Capture a weak pointer of this in the route handler to make sure that:
					//* We don't prevent this from being destroyed (capturing a shared pointer)
					//* If destroyed, we don't try to use it in the handler (capturing a this reference directly)
					std::weak_ptr<ServerPoolsService> wService = this->shared_from_this();
					scene->addRoute("ServerPool.Shutdown", [wService](Stormancer::Packetisp_ptr packet)
					{
						auto service = wService.lock();
						//If service is valid, forward the event.
						if (service)
						{
							service->shutdownReceived();
						}
					});

					auto rpc = _rpcService.lock();

					rpc->addProcedure("ServerPool.GetStatus", [wService](Stormancer::RpcRequestContext_ptr ctx)
					{
						auto service = wService.lock();
						if (!service)
						{
							ctx->sendValueTemplated(Stormancer::ServerPools::Status::Complete);
						}
						if (!service->getStatusCallback)
						{
							ctx->sendValueTemplated(Stormancer::ServerPools::Status::Unknown);
						}
						return pplx::task_from_result();
					});
				}
			};
		}

		class ServerPools : public Stormancer::ClientAPI<ServerPools, details::ServerPoolsService>
		{
			friend class ServerPoolsPlugin;

		public:

			ServerPools(std::weak_ptr<Stormancer::Users::UsersApi> auth)
				: Stormancer::ClientAPI<ServerPools, details::ServerPoolsService>(auth, "stormancer.serverPool")
			{
			}

			template<typename T>
			pplx::task<GameSessionStartupParameters<T>> setReady()
			{
				return this->getService()
					.then([](std::shared_ptr<details::ServerPoolsService> service)
				{
					return service->setReady<T>();
				});
			}

			Stormancer::Event<void> shutdownReceived;

			//Event fired when Stormancer requests a status update from the server
			std::function<Stormancer::ServerPools::Status()> getStatusCallback;

		private:

			void onConnecting(std::shared_ptr <details::ServerPoolsService> service)
			{
				std::weak_ptr<ServerPools> wThis = this->shared_from_this();
				//Always capture weak references, and NEVER 'this'. As the callback is going to be executed asynchronously,
				//who knows what may have happened to the object behind the this pointer since it was captured?
				shutdownReceivedSubscription = service->shutdownReceived.subscribe([wThis]()
				{
					auto that = wThis.lock();
					//If this is valid, forward the event.
					if (that)
					{
						that->shutdownReceived();
					}
				});
				service->getStatusCallback = [wThis]()
				{
					auto that = wThis.lock();
					if (that && that->getStatusCallback)
					{
						return that->getStatusCallback();
					}
					else
					{
						return Stormancer::ServerPools::Status::Unknown;
					}
				};
			}

			void onDisconnecting(std::shared_ptr <details::ServerPoolsService> service)
			{
				//Unsubscribe by destroying the subscription
				shutdownReceivedSubscription = nullptr;
			}

			Stormancer::Event<std::string>::Subscription shutdownReceivedSubscription;
		};

		class ServerPoolsPlugin : public Stormancer::IPlugin
		{
		public:

			void registerSceneDependencies(Stormancer::ContainerBuilder& builder, std::shared_ptr<Stormancer::Scene> scene) override
			{
				auto name = scene->getHostMetadata("stormancer.serverPool");

				if (!name.empty())
				{
					builder.registerDependency<details::ServerPoolsService>([](const Stormancer::DependencyScope& scope) {
						auto instance = std::make_shared<details::ServerPoolsService>(scope.resolve<Stormancer::RpcService>());

						return instance;
					}).singleInstance();
				}
			}

			void registerClientDependencies(Stormancer::ContainerBuilder& builder) override
			{
				builder.registerDependency<ServerPools, Stormancer::Users::UsersApi>().singleInstance();
			}

			void sceneCreated(std::shared_ptr<Scene> scene) override
			{
				auto name = scene->getHostMetadata("stormancer.serverPool");

				if (!name.empty())
				{
					auto service = scene->dependencyResolver().resolve<details::ServerPoolsService>();
					service->initialize(scene);
				}
			}

			void sceneConnecting(std::shared_ptr<Scene> scene) override
			{
				auto name = scene->getHostMetadata("stormancer.serverPool");

				if (!name.empty())
				{
					auto pools = scene->dependencyResolver().resolve<ServerPools>();
					auto service = scene->dependencyResolver().resolve<details::ServerPoolsService>();
					pools->onConnecting(service);
				}
			}

			void sceneDisconnecting(std::shared_ptr<Scene> scene) override
			{
				auto name = scene->getHostMetadata("stormancer.serverPool");

				if (!name.empty())
				{
					auto pools = scene->dependencyResolver().resolve<ServerPools>();
					auto service = scene->dependencyResolver().resolve<details::ServerPoolsService>();
					pools->onDisconnecting(service);
				}
			}
		};
	}
}

MSGPACK_ADD_ENUM(Stormancer::ServerPools::Status);

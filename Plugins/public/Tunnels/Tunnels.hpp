#pragma once

#include "stormancer/ClientAPI.h"
#include "Authentication/AuthenticationService.h"
#include "stormancer/IPlugin.h"
#include "stormancer/Utilities/TaskUtilities.h"

/// <summary>
/// Tunnels plugin to establish a p2p connection with another client connected to a gamesession implementing bfg.tunnels
/// </summary>
/// <remarks>
/// This plugin is intentionnally very simple and does not implement many features and safeguards we would want in this kind of plugin. It's not intended to be used as is in any other game than BFG1.
/// </remarks>
namespace Stormancer
{
	namespace Tunnels
	{
		const std::string TUNNELS_P2P_SERVER_ID = "Bfg.Tunnels";

		class TunnelsPlugin;
		class Tunnels;
		namespace details
		{

			class TunnelsService :public std::enable_shared_from_this<TunnelsService>
			{
				friend class Stormancer::Tunnels::TunnelsPlugin;
				friend class Stormancer::Tunnels::Tunnels;
			public:
				TunnelsService(std::weak_ptr<Scene> scene, std::weak_ptr<RpcService> rpc)
					: _scene(scene)
					, _rpcService(rpc)
				{
				}

				pplx::task<std::shared_ptr<P2PTunnel>> openTunnel()
				{
					std::weak_ptr<TunnelsService> wService = this->shared_from_this();
					return pplx::create_task(_peerTce)
						.then([wService](std::weak_ptr<IP2PScenePeer> wPeer)
					{
						auto service = wService.lock();
						if (!service)
						{
							throw Stormancer::PointerDeletedException("Service destroyed");
						}

						auto rpc = service->_rpcService.lock();
						if (!rpc)
						{
							throw Stormancer::PointerDeletedException("Scene destroyed");
						}

						return rpc->rpc<bool>("Tunnels.OpenP2PTunnel")
							.then([wService, wPeer](bool opened)
						{
							auto service = wService.lock();
							if (!service)
							{
								throw Stormancer::PointerDeletedException("Service destroyed");
							}
							if (!opened)
							{
								return taskDelay(std::chrono::milliseconds(300)).then([wService]()
								{
									auto service = wService.lock();
									if (!service)
									{
										throw Stormancer::PointerDeletedException("Service destroyed");
									}
									return service->openTunnel();
								});
							}
							else
							{								
								auto peer = wPeer.lock();
								if(!peer)
								{
									throw Stormancer::PointerDeletedException("peer destroyed");
								}
								return peer->openP2PTunnel(TUNNELS_P2P_SERVER_ID);								
							}
						});
					});
				}


			private:
				std::weak_ptr<Scene> _scene;
				std::weak_ptr<RpcService> _rpcService;
				std::shared_ptr<P2PTunnel> _tunnel = nullptr;
				Subscription _onPeerConnectedSubscription = nullptr;
				pplx::task_completion_event<std::weak_ptr<IP2PScenePeer>> _peerTce;

				//Initializes the service
				void initialize()
				{
					//Capture a weak pointer of this in the route handler to make sure that:
					//* We don't prevent this from being destroyed (capturing a shared pointer)
					//* If destroyed, we don't try to use it in the handler (capturing a this reference directly)
					std::weak_ptr<TunnelsService> wService = this->shared_from_this();
					auto scene = _scene.lock();
					_onPeerConnectedSubscription = scene->onPeerConnected().subscribe([wService](std::shared_ptr<IP2PScenePeer> peer)
					{
						auto service = wService.lock();
						if (service)
						{
							service->_onPeerConnectedSubscription->unsubscribe();
							service->_peerTce.set(peer);
						}
					});

					auto rpcService = _rpcService.lock();
					if (rpcService)
					{

						rpcService->addProcedure("Tunnels.OpenTunnel", [wService](RpcRequestContext_ptr ctx)
						{
							auto service = wService.lock();
							//If service is valid, open the tunnel
							if (service)
							{
								auto scene = service->_scene.lock();
								if (scene)
								{
									service->_tunnel = scene->registerP2PServer(TUNNELS_P2P_SERVER_ID);
									return pplx::task_from_result();
								}
							}

							return pplx::task_from_exception<void>(Stormancer::PointerDeletedException("Scene destroyed"));
						});
					}
				}

				void onDisconnecting()
				{
					_tunnel = nullptr;
				}
			};

		}

		class Tunnels
		{
			friend class TunnelsPlugin;
		public:


			pplx::task<std::shared_ptr<P2PTunnel>> openTunnel()
			{
				if (_service)
				{
					return _service->openTunnel();
				}
				else
				{
					return pplx::task_from_exception<std::shared_ptr<P2PTunnel>>(std::runtime_error("No connected to game session implementing bfg.tunnels"));
				}
			}


		private:
			void onConnecting(std::shared_ptr< details::TunnelsService> service)
			{
				_service = service;
			}
			void onDisconnecting(std::shared_ptr< details::TunnelsService> service)
			{
				if (_service)
				{
					_service->onDisconnecting();
				}
				_service = nullptr;
			}

			std::shared_ptr<details::TunnelsService> _service;
		};

		class TunnelsPlugin : public Stormancer::IPlugin
		{
		public:

			void registerSceneDependencies(Stormancer::ContainerBuilder& builder, std::shared_ptr<Stormancer::Scene> scene) override
			{

				auto name = scene->getHostMetadata("bfg.tunnels");

				if (!name.empty())
				{
					builder.registerDependency<details::TunnelsService, Scene, RpcService>().singleInstance();
				}

			}
			void registerClientDependencies(Stormancer::ContainerBuilder& builder) override
			{
				builder.registerDependency<Tunnels>().singleInstance();
			}

			void sceneCreated(std::shared_ptr<Scene> scene) override
			{
				auto name = scene->getHostMetadata("bfg.tunnels");

				if (!name.empty())
				{
					auto service = scene->dependencyResolver().resolve<details::TunnelsService>();
					service->initialize();
				}
			}

			void sceneConnecting(std::shared_ptr<Scene> scene) override
			{
				auto name = scene->getHostMetadata("bfg.tunnels");

				if (!name.empty())
				{
					auto hello = scene->dependencyResolver().resolve<Tunnels>();
					auto service = scene->dependencyResolver().resolve<details::TunnelsService>();
					hello->onConnecting(service);
				}
			}

			void sceneDisconnecting(std::shared_ptr<Scene> scene) override
			{
				auto name = scene->getHostMetadata("bfg.tunnels");

				if (!name.empty())
				{
					auto tunnels = scene->dependencyResolver().resolve<Tunnels>();
					auto service = scene->dependencyResolver().resolve<details::TunnelsService>();
					tunnels->onDisconnecting(service);
				}
			}
		};

	}

}
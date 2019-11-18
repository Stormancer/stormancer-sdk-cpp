#pragma once
#include "stormancer/IPlugin.h"
#include "stormancer/Tasks.h"
#include "stormancer/Scene.h"
#include "Users/Users.hpp"
#include "stormancer/ITokenHandler.h"
#include "stormancer/IClient.h"

namespace Stormancer
{

	namespace Status
	{
		enum class ServiceState
		{
			Unknown,
			Online,
			Offline
		};

		class ServiceMonitor
		{
			virtual void monitor(std::string serviceType, std::string serviceName) = 0;
			virtual void stopMonitoring(std::string serviceType, std::string serviceName) = 0;

			virtual ServiceState getStatus(std::string serviceType, std::string serviceName) = 0;

		};
		class ServiceStatusPlugin;

		namespace details
		{

			class ServiceMonitor_Impl : public ServiceMonitor, public std::enable_shared_from_this<ServiceMonitor_Impl>
			{
				friend class Stormancer::Status::ServiceStatusPlugin;
			public:
				ServiceMonitor_Impl(
					std::shared_ptr<Stormancer::Users::UsersApi> users,
					std::shared_ptr<ITokenHandler> tokens,
					std::shared_ptr<IClient> client)
				{
					_client = client;
					_tokens = tokens;
					_users = users;
				}
				virtual ~ServiceMonitor_Impl()
				{

				}
			private:


				// Inherited from ServiceMonitor
				virtual void monitor(std::string serviceType, std::string serviceName) final
				{
					getStatus(serviceType, serviceName);
				}

				virtual void stopMonitoring(std::string serviceType, std::string serviceName) final
				{
					auto serviceId = serviceType + "/" + serviceName;
					auto it = _serviceIdsMappings.find(serviceId);
					if (it != _serviceIdsMappings.end())
					{
						_connectionStates.erase(it->second);

					}
					_serviceIdsMappings.erase(serviceId);
				}

				// Inherited from ServiceMonitor
				virtual ServiceState getStatus(std::string serviceType, std::string serviceName) final
				{
					auto serviceId = serviceType + "/" + serviceName;
					auto it = _serviceIdsMappings.find(serviceId);
					std::string sceneId;
					if (it != _serviceIdsMappings.end())
					{
						sceneId = it->second;
					}
					else
					{
						auto users = _users.lock();
						
						if (users)
						{
							auto wClient = _client;
							auto wTokens = _tokens;

							std::weak_ptr<ServiceMonitor_Impl> wThis = this->shared_from_this();
							users->getSceneConnectionToken(serviceType, serviceName, pplx::cancellation_token::none()).then([serviceId, wThis, wTokens, wClient](pplx::task<std::string> t) {

								auto that = wThis.lock();
								auto tokens = wTokens.lock();
								auto client = wClient.lock();
								if (that && tokens && client)
								{
									auto token = t.get();
									auto endpoint = tokens->decodeToken(token);
									auto sceneId = endpoint.tokenData.SceneId;
									that->_serviceIdsMappings[serviceId] = sceneId;
									return client->connectToPrivateScene(token).then([sceneId,wThis](pplx::task<std::shared_ptr<Scene>> t) {
										auto that = wThis.lock();
										try
										{
											if (that)
											{
												that->_connectionStates[sceneId] = t.get();
											}
										}
										catch (std::exception&)
										{
											
											if (that)
											{
												that->_connectionStates[sceneId] = nullptr;
											}
										}
									});
								}
								else
								{
									return pplx::task_from_result();
								}

							}).then([](pplx::task<void> t) {
								try
								{
									t.get();
								}
								catch (std::exception&)
								{

								}
							});
						}
						return ServiceState::Unknown;
					}


					auto statusIt = _connectionStates.find(sceneId);
					if (statusIt == _connectionStates.end())
					{

						return ServiceState::Unknown;
					}
					else
					{
						if (statusIt->second)
						{
							return ServiceState::Online;
						}
						else
						{
							return ServiceState::Offline;
						}
					}


				}


				void onConnected(std::shared_ptr<Scene> scene)
				{
					_connectionStates[scene->id()] = scene;
				}

				void onDisconnecting(std::shared_ptr<Scene> scene)
				{
					_connectionStates[scene->id()] = nullptr;
				}



			private:

				std::weak_ptr<Stormancer::Users::UsersApi> _users;
				std::weak_ptr<Stormancer::ITokenHandler> _tokens;
				std::weak_ptr<Stormancer::IClient> _client;
				std::unordered_map <std::string, std::shared_ptr<Scene>> _connectionStates;
				/// <summary>
				/// mapping serviceId => sceneId
				/// </summary>
				std::unordered_map<std::string, std::string> _serviceIdsMappings;


			};
		}



		class ServiceStatusPlugin : public IPlugin
		{
		public:
			void registerClientDependencies(Stormancer::ContainerBuilder& builder) override
			{
				builder.registerDependency<details::ServiceMonitor_Impl, Stormancer::Users::UsersApi, Stormancer::ITokenHandler, Stormancer::IClient>().as<ServiceMonitor>().singleInstance();
			}

			void sceneDisconnecting(std::shared_ptr<Scene> scene) override
			{

				auto impl = std::static_pointer_cast<details::ServiceMonitor_Impl>(scene->dependencyResolver().resolve<ServiceMonitor>());
				impl->onDisconnecting(scene);

			}

			void sceneConnected(std::shared_ptr<Scene> scene) override
			{
				auto impl = std::static_pointer_cast<details::ServiceMonitor_Impl>(scene->dependencyResolver().resolve<ServiceMonitor>());
				impl->onConnected(scene);
			}

		};
	}
}

#pragma once

#include "stormancer/Scene.h"
#include "Users/Users.hpp"

namespace Stormancer
{
	template<typename TManager, typename TService>
	class ClientAPI : public std::enable_shared_from_this<TManager>
	{
	protected:

		ClientAPI(std::weak_ptr<Users::UsersApi> users, const std::string& type = "", const std::string& name = "")
			: _users(users)
			, _type(type)
			, _name(name)
		{
		}

		std::weak_ptr<TManager> weak_from_this()
		{
			return this->shared_from_this();
		}

		pplx::task<std::shared_ptr<TService>> getService(
			std::function<void(std::shared_ptr<TManager>, std::shared_ptr<TService>, std::shared_ptr<Scene>)> initializer = [](auto that, auto service, auto scene) {},
			std::function<void(std::shared_ptr<TManager>, std::shared_ptr<Scene>)> cleanup = [](auto that, auto scene) {}
		)
		{
			if (!_serviceTask)
			{
				auto users = _users.lock();
				if (!users)
				{
					STORM_RETURN_TASK_FROM_EXCEPTION(std::shared_ptr<TService>, std::runtime_error("destroyed"));
				}

				std::weak_ptr<TManager> wThat = this->shared_from_this();

				if (!_scene)
				{
					_scene = std::make_shared<pplx::task<std::shared_ptr<Scene>>>(users->getSceneForService(_type, _name)
						.then([wThat, cleanup](std::shared_ptr<Scene> scene)
					{
						auto that = wThat.lock();
						if (!that)
						{
							throw std::runtime_error("destroyed");
						}

						std::weak_ptr<Scene> wScene = scene;
						that->_connectionChangedSub = scene->getConnectionStateChangedObservable().subscribe([wThat, wScene, cleanup](ConnectionState state)
						{
							auto that = wThat.lock();
							if (!that)
							{
								throw std::runtime_error("destroyed");
							}

							if (state == ConnectionState::Disconnected || state == ConnectionState::Disconnecting)
							{
								cleanup(that, wScene.lock());
								if (that->_connectionChangedSub.is_subscribed())
								{
									that->_connectionChangedSub.unsubscribe();
								}
								that->_scene = nullptr;
								that->_serviceTask = nullptr;
							}
						});
						if (scene->getCurrentConnectionState() == ConnectionState::Disconnected || scene->getCurrentConnectionState() == ConnectionState::Disconnecting)
						{
							cleanup(that, scene);

							if (that->_connectionChangedSub.is_subscribed())
							{
								that->_connectionChangedSub.unsubscribe();
							}
							that->_scene = nullptr;
							that->_serviceTask = nullptr;
						}
						return scene;
					})
						.then([wThat, cleanup](pplx::task<std::shared_ptr<Scene>> t)
					{
						try
						{
							return t.get();
						}
						catch (std::exception&)
						{
							auto that = wThat.lock();
							if (!that)
							{
								throw std::runtime_error("destroyed");
							}

							cleanup(that, nullptr);
							if (that->_connectionChangedSub.is_subscribed())
							{
								that->_connectionChangedSub.unsubscribe();
							}
							that->_scene = nullptr;
							that->_serviceTask = nullptr;
							throw;
						}
					}));
				}

				auto taskService = _scene->then([wThat, initializer](std::shared_ptr<Scene> scene)
				{
					auto service = scene->dependencyResolver().resolve<TService>();
					auto that = wThat.lock();
					if (!that)
					{
						throw std::runtime_error("destroyed");
					}
					initializer(that, service, scene);

					return service;
				});

				_serviceTask = std::make_shared<pplx::task<std::shared_ptr<TService>>>(taskService);
			}

			if (!_serviceTask)
			{
				STORM_RETURN_TASK_FROM_EXCEPTION(std::shared_ptr<TService>, std::runtime_error("service not found"));
			}

			return *_serviceTask;
		}

	protected:

		std::weak_ptr<Users::UsersApi> _users;
		
		std::string _type;

		std::string _name;

	private:

		std::shared_ptr<pplx::task<std::shared_ptr<Scene>>> _scene;

		std::shared_ptr<pplx::task<std::shared_ptr<TService>>> _serviceTask;

		rxcpp::subscription _connectionChangedSub;
	};
}

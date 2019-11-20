#pragma once

#include "stormancer/Scene.h"
#include "Users/Users.hpp"

namespace Stormancer
{
	template<typename TManager>
	class ClientAPI : public std::enable_shared_from_this<TManager>
	{
	protected:

		ClientAPI(std::weak_ptr<Users::UsersApi> users) : _users(users)
		{
		}

		std::weak_ptr<TManager> weak_from_this()
		{
			return this->shared_from_this();
		}

		template<class TService>
		pplx::task<std::shared_ptr<TService>> getService(std::string type,
			std::function < void(std::shared_ptr<TManager>, std::shared_ptr<TService>, std::shared_ptr<Scene>)> initializer = [](auto that, auto service, auto scene) {},
			std::function<void(std::shared_ptr<TManager>, std::shared_ptr<Scene>)> cleanup = [](auto that, auto scene) {},
			std::string name = "")
		{
			auto serviceTaskIt = _serviceTasks.find(type);

			if (serviceTaskIt == _serviceTasks.end())
			{
				auto users = _users.lock();
				if (!users)
				{
					return pplx::task_from_exception<std::shared_ptr<TService>>(std::runtime_error("destroyed"));
				}

				std::weak_ptr<TManager> wThat = this->shared_from_this();

				if (!_scene)
				{
					_scene = std::make_shared<pplx::task<std::shared_ptr<Scene>>>(users->getSceneForService(type, name)
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
								that->_serviceTasks.clear();
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
							that->_serviceTasks.clear();
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
							that->_serviceTasks.clear();
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

				std::shared_ptr<void> taskServiceSharedPtr(static_cast<void*>(new pplx::task<std::shared_ptr<TService>>(taskService)));
				_serviceTasks.emplace(type, taskServiceSharedPtr);
				serviceTaskIt = _serviceTasks.find(type);
			}

			if (!serviceTaskIt->second)
			{
				STORM_RETURN_TASK_FROM_EXCEPTION(std::shared_ptr<TService>, std::runtime_error("Empty service task"));
			}
			auto ptr = serviceTaskIt->second.get();
			auto taskPtr = static_cast<pplx::task<std::shared_ptr<TService>>*>(ptr);
			return *taskPtr;
		}

	protected:

		std::weak_ptr<Users::UsersApi> _users;

	private:

		std::shared_ptr<pplx::task<std::shared_ptr<Scene>>> _scene;

		std::unordered_map<std::string, std::shared_ptr<void>> _serviceTasks;

		rxcpp::subscription _connectionChangedSub;
	};
}

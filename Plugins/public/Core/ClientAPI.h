#pragma once
#include "stormancer/headers.h"
#include "stormancer/Scene.h"
#include  "Authentication/AuthenticationService.h"

namespace Stormancer
{
	template<typename TManager>
	class ClientAPI : public std::enable_shared_from_this<TManager>
	{
	protected:

		ClientAPI(std::weak_ptr<AuthenticationService> auth) :_auth(auth) {}


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
			auto auth = _auth.lock();
			std::weak_ptr<TManager> wThat = this->shared_from_this();
			if (!auth)
			{
				return pplx::task_from_exception<std::shared_ptr<TService>>(std::runtime_error("destroyed"));
			}
			if (!_scene)
			{


				_scene = std::make_shared<pplx::task<std::shared_ptr<Scene>>>(auth->getSceneForService(type, name).then([wThat, cleanup](std::shared_ptr<Scene> scene) {
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
					}
					return scene;
				}).then([wThat, cleanup](pplx::task<std::shared_ptr<Scene>> t) {
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
						throw;
					}

				}));

			}

			return _scene->then([wThat, initializer](std::shared_ptr<Scene> scene) {

				auto service = scene->dependencyResolver()->resolve<TService>();
				auto that = wThat.lock();
				if (!that)
				{
					throw std::runtime_error("destroyed");
				}
				initializer(that, service, scene);

				return service;
			});
		}
	protected:
		std::weak_ptr<AuthenticationService> _auth;

	private:

		std::shared_ptr<pplx::task<std::shared_ptr<Scene>>> _scene;

		rxcpp::subscription _connectionChangedSub;
		
	};
}
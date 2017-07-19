#pragma once
#include <headers.h>
#include <Configuration.h>
#include <Client.h>
#include "MatchMaking/MatchMakingService.h"
#include "Authentication/AuthenticationService.h"

namespace Stormancer
{
	enum MatchType
	{
		Fast,
		Ranked
	};

	class StormancerWrapper
	{
	public:

#pragma region public_methods

		StormancerWrapper();
		~StormancerWrapper();

		StormancerWrapper(const StormancerWrapper&) = delete;
		StormancerWrapper& operator=(const StormancerWrapper&) = delete;

		void init(const std::string& endpoint, const std::string& account, const std::string& application);
		void update();
		void shutdown(bool immediate = false);

		pplx::task<void> Authenticate(const std::string& provider, const std::string& authenticationToken);

		std::shared_ptr<IActionDispatcher> getDispatcher();

		void setLogger(std::shared_ptr<ILogger> logger);
		ILogger_ptr getLogger();

		pplx::task<void> lauchMatchmaking(MatchType type);
		pplx::task<void> cancelMatchmaking(MatchType type);
		void onMatchFound(const std::function<void(MatchmakingResponse)>& matchFoundCallback);
		void onMatchUpdate(const std::function<void(MatchState)>& matchUpdateCallback);

		template<typename T>
		pplx::task<std::shared_ptr<T>> getService(const std::string& sceneId, bool needAuthentication = true, bool connect = true)
		{
			pplx::task_completion_event<std::shared_ptr<T>> tce;
			auto scene = _scenes[sceneId];
			if (scene)
			{
				auto service = scene->dependencyResolver()->resolve<T>();
				tce.set(service);
				return pplx::create_task(tce, pplx::task_options(_actionDispatcher));
			}

			pplx::task<Scene_ptr> sceneTask;
			if (needAuthentication)
			{
				_authService = _client->dependencyResolver()->resolve<AuthenticationService>();
				if (_authService->connectionState() != GameConnectionState::Authenticated)
				{
					tce.set_exception(std::runtime_error("User not authenticated."));
					return pplx::create_task(tce, pplx::task_options(_actionDispatcher));
				}
				sceneTask = _authService->getPrivateScene(sceneId);
			}
			else
			{
				sceneTask = _client->getPublicScene(sceneId);
			}

			sceneTask.then([tce, this, connect](pplx::task<Scene_ptr> task) {
				try
				{
					auto scene = task.get();
					(connect ? scene->connect() : pplx::task_from_result()).then([this, scene, tce](pplx::task<void> connectTask) {
						try
						{
							connectTask.get();

							_scenes[scene->id()] = scene;
							auto service = scene->dependencyResolver()->resolve<T>();
							assert(service);
							tce.set(service);
						}
						catch (std::exception ex)
						{
							tce.set_exception(ex);
							return;
						}
					});
				}
				catch (std::exception ex)
				{
					tce.set_exception(ex);
					return;
				}
			});

			return pplx::create_task(tce, pplx::task_options(_actionDispatcher));
		}

		static std::shared_ptr<StormancerWrapper> instance();
		static void setInstance(std::shared_ptr<StormancerWrapper> wrapper);

#pragma endregion

	private:

#pragma region private_members

		std::string _endpoint;
		std::string _account;
		std::string _application;
		Configuration_ptr _config;
		Client_ptr _client;
		std::unordered_map<std::string, Scene_ptr> _scenes;
		ILogger_ptr _logger;
		std::shared_ptr<AuthenticationService> _authService;
		std::shared_ptr<MainThreadActionDispatcher> _actionDispatcher;
		std::function<void(MatchmakingResponse)> _onMatchFound;
		std::function<void(MatchState)> _onMatchUpdate;
		std::function<void()> _onMatchParameterUpdate;

		static std::shared_ptr<StormancerWrapper> _instance;

#pragma endregion
	};
}

#pragma once

#include "stormancer/headers.h"
#include "stormancer/Configuration.h"
#include "stormancer/Client.h"
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
		void shutdown();

		pplx::task<void> Authenticate(const std::string& provider, const std::string& authenticationToken);
		std::string userId();

		std::shared_ptr<IActionDispatcher> getDispatcher();

		void setLogger(std::shared_ptr<ILogger> logger);
		ILogger_ptr getLogger();

		pplx::task<void> lauchMatchmaking(MatchType type, const MatchmakingRequest& mmRequest);
		pplx::task<void> cancelMatchmaking(MatchType type);
		pplx::task<void> resolveMatchmaking(MatchType type, bool acceptMatch);

		void onMatchFound(const std::function<void(MatchmakingResponse)>& matchFoundCallback);
		void onMatchUpdate(const std::function<void(MatchState)>& matchUpdateCallback);
		void onMatchReadyUpdate(const std::function<void(ReadyVerificationRequest)>& matchReadyUpdateCallback);

		std::string getMatchMakingSceneName(MatchType type);

		template<typename T>
		pplx::task<std::shared_ptr<T>> getService(const std::string& sceneId, bool needAuthentication = true, bool connect = true)
		{
			pplx::task_completion_event<std::shared_ptr<T>> tce;

			pplx::task<Scene_ptr> sceneTask;
			if (needAuthentication)
			{
				auto authService = _client->dependencyResolver()->resolve<AuthenticationService>();
				if (authService->connectionState() != GameConnectionState::Authenticated)
				{
					tce.set_exception(std::runtime_error("User not authenticated."));
					return pplx::create_task(tce, pplx::task_options(_actionDispatcher));
				}
				sceneTask = authService->getPrivateScene(sceneId);
			}
			else
			{
				sceneTask = _client->getPublicScene(sceneId);
			}

			sceneTask.then([this, connect, tce](pplx::task<Scene_ptr> task) {
				try
				{
					auto scene = task.get();
					pplx::task<void> connectTask;
					if (connect)
					{
						connectTask = scene->connect();
					}
					else
					{
						connectTask = pplx::task_from_result();
					}
					connectTask
						.then([this, scene, tce](pplx::task<void> connectTask)
					{
						try
						{
							connectTask.get();
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
		ILogger_ptr _logger;
		std::shared_ptr<MainThreadActionDispatcher> _actionDispatcher;

		std::function<void(MatchmakingResponse)> _onMatchFoundCallback;
		std::function<void(MatchState)> _onMatchUpdateCallback;
		std::function<void(ReadyVerificationRequest)> _onMatchReadyUpdateCallback;

		static std::shared_ptr<StormancerWrapper> _instance;

#pragma endregion
	};
}

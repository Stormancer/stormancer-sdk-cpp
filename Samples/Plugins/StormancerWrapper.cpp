#include "stormancer/Logger/ConsoleLogger.h"
#include "stormancer/SingleServicePlugin.h"
#include "StormancerWrapper.h"
#include "Authentication/AuthenticationPlugin.h"
#include "BugReport/BugReportPlugin.h"
#include "GameSession/GameSessionPlugin.h"
#include "GameVersion/GameVersionPlugin.h"
#include "Matchmaking/MatchmakingPlugin.h"
#include "PlayerProfile/PlayerProfilePlugin.h"
#include "PlayerProfile/PlayerProfile.h"
#include "Leaderboard/LeaderboardPlugin.h"
#include "Leaderboard/LeaderboardService.h"

namespace Stormancer
{

	namespace
	{
		ServiceOptions playerProfileServiceOptions{
			ServiceContextFlags::Scene | ServiceContextFlags::CreateWithScene | ServiceContextFlags::SingleInstance,
			"profiles"
		};
	}

	std::shared_ptr<StormancerWrapper> StormancerWrapper::_instance;

	StormancerWrapper::StormancerWrapper()
		: _actionDispatcher(std::make_shared<MainThreadActionDispatcher>())
	{
	}

	StormancerWrapper::~StormancerWrapper()
	{
		shutdown();
	}

	void StormancerWrapper::init(const std::string& endpoint, const std::string& account, const std::string& application)
	{
		_endpoint = endpoint;
		_account = account;
		_application = application;
		_config = Configuration::create(_endpoint, _account, _application);
		_config->logger = _logger;
		_config->endpointSelectionMode = EndpointSelectionMode::FALLBACK;
		_config->actionDispatcher = _actionDispatcher;
		_config->addPlugin(new AuthenticationPlugin());
		_config->addPlugin(new BugReportPlugin());
		_config->addPlugin(new GameSessionPlugin());
		_config->addPlugin(new GameVersionPlugin());
		_config->addPlugin(new LeaderboardPlugin());
		_config->addPlugin(new MatchmakingPlugin());

		_config->addPlugin(new SingleServicePlugin<PlayerProfileService<PlayerProfile>, playerProfileServiceOptions>());

		_client = Client::create(_config);
	}

	void StormancerWrapper::update()
	{
		if (_actionDispatcher)
		{
			_actionDispatcher->update(std::chrono::milliseconds(10));
		}
	}

	void StormancerWrapper::shutdown()
	{
		if (_client && _client->getConnectionState() == ConnectionState::Connected)
		{
			_client->disconnect().wait();
			_actionDispatcher.reset();
			_client.reset();
			_config.reset();
		}
	}

	pplx::task<void> StormancerWrapper::Authenticate(const std::string& provider, const std::string& authenticationToken)
	{
		auto authService = _client->dependencyResolver().lock()->resolve<AuthenticationService>();

		const std::map<std::string, std::string> authenticationContext{
			{ "provider", provider },
			{ "ticket", authenticationToken }
		};

		return authService->login(authenticationContext);
	}

	std::string StormancerWrapper::userId()
	{
		auto authService = _client->dependencyResolver().lock()->resolve<AuthenticationService>();

		return authService->userId();
	}

	std::shared_ptr<IActionDispatcher> StormancerWrapper::getDispatcher()
	{
		return _client->dependencyResolver().lock()->resolve<IActionDispatcher>();
	}

	void StormancerWrapper::setLogger(std::shared_ptr<ILogger> logger)
	{
		_logger = logger;
	}

	ILogger_ptr StormancerWrapper::getLogger()
	{
		return _logger;
	}

	pplx::task<void> StormancerWrapper::lauchMatchmaking(MatchType type, const MatchmakingRequest& mmRequest)
	{
		std::string matchmakingScene = getMatchMakingSceneName(type);

		return getService<MatchmakingService>(matchmakingScene).then([=](std::shared_ptr<MatchmakingService> matchmakingService)
		{
			matchmakingService->onMatchFound(_onMatchFoundCallback);
			matchmakingService->onMatchUpdate(_onMatchUpdateCallback);
			matchmakingService->onMatchReadyUpdate(_onMatchReadyUpdateCallback);
			return matchmakingService->findMatch("game", mmRequest);
		});
	}

	pplx::task<void> StormancerWrapper::cancelMatchmaking(MatchType type)
	{
		std::string matchmakingScene = getMatchMakingSceneName(type);

		return getService<MatchmakingService>(matchmakingScene).then([=](std::shared_ptr<MatchmakingService> matchmakingService)
		{
			if (matchmakingService)
			{
				matchmakingService->cancel();
			}
		});
	}

	pplx::task<void> StormancerWrapper::resolveMatchmaking(MatchType type, bool acceptMatch)
	{
		std::string matchmakingScene = getMatchMakingSceneName(type);

		return getService<MatchmakingService>(matchmakingScene).then([=](std::shared_ptr<MatchmakingService> matchmakingService)
		{
			if (matchmakingService)
			{
				matchmakingService->resolve(acceptMatch);
			}
		});
	}

	void StormancerWrapper::onMatchFound(const std::function<void(MatchmakingResponse)>& matchFoundCallback)
	{
		_onMatchFoundCallback = matchFoundCallback;
	}

	void StormancerWrapper::onMatchUpdate(const std::function<void(MatchState)>& matchUpdateCallback)
	{
		_onMatchUpdateCallback = matchUpdateCallback;
	}

	void StormancerWrapper::onMatchReadyUpdate(const std::function<void(ReadyVerificationRequest)>& matchReadyUpdateCallback)
	{
		_onMatchReadyUpdateCallback = matchReadyUpdateCallback;
	}

	std::string StormancerWrapper::getMatchMakingSceneName(MatchType type)
	{
		std::string matchmakingScene;

		switch (type)
		{
		case Fast:
			matchmakingScene = "matchmaker-fast";
			break;
		case Ranked:
			matchmakingScene = "matchmaker-ranked";
			break;
		default:
			throw std::runtime_error("invalid match type");
			break;
		}

		return matchmakingScene;
	}

	std::shared_ptr<StormancerWrapper> StormancerWrapper::instance()
	{
		return StormancerWrapper::_instance;
	}

	void StormancerWrapper::setInstance(std::shared_ptr<StormancerWrapper> wrapper)
	{
		StormancerWrapper::_instance = wrapper;
	}
}

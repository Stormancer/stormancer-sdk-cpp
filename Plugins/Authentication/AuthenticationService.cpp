#include "stormancer/headers.h"
#include "stormancer/RPC/RpcService.h"
#include "AuthenticationService.h"

namespace Stormancer
{
	AuthenticationService::AuthenticationService(Client* client)
		: _client(client)
		, _logger(client->logger())
	{
		auto onNext = [this](ConnectionState state) {
			switch (state)
			{
			case ConnectionState::Connecting:
			case ConnectionState::Disconnecting:
			case ConnectionState::Disconnected:
				setConnectionState((GameConnectionState)state);
				break;
			default:
				break;
			}
		};

		auto onError = [this](std::exception_ptr exptr) {
			try
			{
				std::rethrow_exception(exptr);
			}
			catch (const std::exception& ex)
			{
				_logger->log(LogLevel::Error, "AuthenticationService", "Client connection state change failed", ex.what());
			}
		};

		_connectionSubscription = client->getConnectionStateChangedObservable().subscribe(onNext, onError);
	}

	AuthenticationService::~AuthenticationService()
	{
		_connectionSubscription.unsubscribe();
	}

	std::string AuthenticationService::authenticationSceneName()
	{
		return _authenticationSceneName;
	}

	void AuthenticationService::setAuthenticationSceneName(const std::string& name)
	{
		_authenticationSceneName = name;
	}

	std::string AuthenticationService::createUserRoute()
	{
		return _createUserRoute;
	}

	void AuthenticationService::setCreateUserRoute(const std::string& name)
	{
		_createUserRoute = name;
	}

	std::string AuthenticationService::loginRoute()
	{
		return _loginRoute;
	}

	void AuthenticationService::setLoginRoute(const std::string& name)
	{
		_loginRoute = name;
	}

	

	pplx::task<void> AuthenticationService::loginSteam(const std::string& ticket)
	{
		std::map<std::string, std::string> authContext{ { "provider", "steam" }, { "ticket", ticket } };
		return login(authContext);
	}

















	

	pplx::task<void> AuthenticationService::login(const std::map<std::string, std::string>& authenticationContext)
	{
		if (_authenticated)
		{
			return pplx::task_from_exception<void>(std::runtime_error("Authentication service already authenticated."));
		}

		return getAuthenticationScene()
			.then([this, authenticationContext](Scene_ptr scene)
		{
			setConnectionState(GameConnectionState::Authenticating);
			auto rpcService = scene->dependencyResolver().lock()->resolve<RpcService>();

			pplx::task_completion_event<void> tce;

			rpcService->rpc<LoginResult, std::map<std::string, std::string>>(_loginRoute, authenticationContext)
				.then([this, tce](pplx::task<LoginResult> task)
			{
				try
				{
					auto loginResult = task.get();
					if (loginResult.success)
					{
						_userId = loginResult.userId;
						_username = loginResult.username;
						setConnectionState(GameConnectionState::Authenticated);
					}
					else
					{
						setConnectionState(GameConnectionState::Disconnected);
						throw std::runtime_error(loginResult.errorMsg);
					}
					tce.set();
				}
				catch (const std::exception& ex)
				{
					tce.set_exception(ex);
				}
			});

			return pplx::create_task(tce);
		});
	}

	pplx::task<Scene_ptr> AuthenticationService::getAuthenticationScene()
	{
		if (_client)
		{
			return _client->connectToPublicScene(_authenticationSceneName)
				.then([](Scene_ptr scene)
			{
				if (scene->getCurrentConnectionState() != ConnectionState::Connected)
				{
					throw std::runtime_error("Scene is not connected.");
				}
				return scene;
			});
		}
		else
		{
			return pplx::task_from_exception<Scene_ptr>(std::runtime_error("Client is invalid."));
		}
	}

	pplx::task<void> AuthenticationService::logout()
	{
		if (_authenticated)
		{
			_authenticated = false;
			return getAuthenticationScene()
				.then([](Scene_ptr scene)
			{
				return scene->disconnect();
			});
		}
		else
		{
			return pplx::task_from_result();
		}
	}

	std::string AuthenticationService::userId()
	{
		return _userId;
	}

	pplx::task<std::string> AuthenticationService::getBearerToken()
	{
		return getAuthenticationScene()
			.then([](Scene_ptr authScene)
		{
			auto rpcService = authScene->dependencyResolver().lock()->resolve<RpcService>();
			return rpcService->rpc<std::string>("sceneauthorization.getbearertoken");
		});
	}

	pplx::task<std::string> AuthenticationService::getUserFromBearerToken(std::string token)
	{
		return getAuthenticationScene()
			.then([token](Scene_ptr authScene)
		{
			auto rpcService = authScene->dependencyResolver().lock()->resolve<RpcService>();
			return rpcService->rpc<std::string, std::string>("sceneauthorization.getuserfrombearertoken", token);
		});
	}

	std::string AuthenticationService::getUsername()
	{
		return _username;
	}

	pplx::task<std::string> AuthenticationService::getUserIdByPseudo(std::string pseudo)
	{
		return getAuthenticationScene()
			.then([pseudo](Scene_ptr authScene)
		{
			auto rpcService = authScene->dependencyResolver().lock()->resolve<RpcService>();
			return rpcService->rpc<std::string, std::string>("users.getuseridbypseudo", pseudo);
		});
	}

	pplx::task<Scene_ptr> AuthenticationService::connectToPrivateScene(const std::string& sceneId, std::function<void(Scene_ptr)> builder)
	{
		std::weak_ptr<AuthenticationService> wThat = this->shared_from_this();
		return getAuthenticationScene()
			.then([sceneId](Scene_ptr authScene)
		{
			auto rpcService = authScene->dependencyResolver().lock()->resolve<RpcService>();
			return rpcService->rpc<std::string, std::string>("sceneauthorization.gettoken", sceneId);
		})
			.then([wThat,builder](std::string token)
		{
			auto that = wThat.lock();
			
			if (that && that->_client)
			{
				return that->_client->connectToPrivateScene(token,builder);
			}
			else
			{
				throw std::runtime_error("Client is invalid.");
			}
		});
	}

	Action<GameConnectionState>& AuthenticationService::connectionStateChangedAction()
	{
		return _onConnectionStateChanged;
	}

	Action<GameConnectionState>::TIterator AuthenticationService::onConnectionStateChanged(const std::function<void(GameConnectionState)>& callback)
	{
		return _onConnectionStateChanged.push_back(callback);
	}

	pplx::task<std::unordered_map<std::string, std::string>> AuthenticationService::getPseudos(std::vector<std::string> userIds)
	{
		return getAuthenticationScene()
			.then([userIds](Scene_ptr authScene)
		{
			auto rpcService = authScene->dependencyResolver().lock()->resolve<RpcService>();
			return rpcService->rpc<std::unordered_map<std::string, std::string>, std::vector<std::string>>("users.getpseudos", userIds);
		});
	}

	GameConnectionState AuthenticationService::connectionState() const
	{
		return _connectionState;
	}

	void AuthenticationService::setConnectionState(GameConnectionState state)
	{
		if (_connectionState != state)
		{
			_connectionState = state;
			_onConnectionStateChanged(state);
		}
	}

	pplx::task<void> AuthenticationService::impersonate(const std::string& provider, const std::string& claimPath, const std::string& uid, const std::string& secret)
	{
		std::map<std::string, std::string> authContext{
			{ "provider", "impersonation" },
			{ "secret", secret } ,
			{"impersonated-provider",provider},
			{"claimPath",claimPath},
			{"claimValue",uid} };
		return login(authContext);
	}
};

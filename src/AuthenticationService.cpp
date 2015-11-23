#include "stormancer.h"

namespace Stormancer
{
	LoginResult::LoginResult(Stormancer::bytestream* stream)
	{
		std::string buffer;
		*stream >> buffer;
		msgpack::unpacked result;
		msgpack::unpack(&result, buffer.data(), buffer.size());
		result.get().convert(this);
	}



	AuthenticationService::AuthenticationService(Client* client)
		: _client(client)
	{
	}

	AuthenticationService::~AuthenticationService()
	{
	}

	const char* AuthenticationService::authenticationSceneName()
	{
		return _authenticationSceneName.c_str();
	}

	void AuthenticationService::setAuthenticationSceneName(const char* name)
	{
		_authenticationSceneName = name;
	}

	const char* AuthenticationService::createUserRoute()
	{
		return _createUserRoute.c_str();
	}

	void AuthenticationService::setCreateUserRoute(const char* name)
	{
		_createUserRoute = name;
	}

	const char* AuthenticationService::loginRoute()
	{
		return _loginRoute.c_str();
	}

	void AuthenticationService::setLoginRoute(const char* name)
	{
		_loginRoute = name;
	}

	pplx::task<Scene*> AuthenticationService::login(const char* pseudo, const char* password)
	{
		stringMap authContext{ { "provider", "loginpassword" }, { pseudo, password } };
		return login(&authContext);
	}

	pplx::task<Scene*> AuthenticationService::steamLogin(const char* steamTicket)
	{
		stringMap authContext{ { "provider", "steam" }, { "ticket", steamTicket } };
		return login(&authContext);
	}

	stringMap processToDll(const stringMap* map)
	{
		stringMap map2;
		for (auto it : *map)
		{
			map2[it.first.c_str()] = it.second.c_str();
		}
		return map2;
	}

	pplx::task<Scene*> AuthenticationService::login(const stringMap* authenticationContext)
	{
		auto authContext = processToDll(authenticationContext);

		if (_authenticated)
		{
			throw std::runtime_error("Authentication service already authenticated.");
		}

		pplx::task_completion_event<Scene*> tce;
		getAuthenticationScene().then([this, authContext, tce](Scene* scene) {
			auto rpcService = scene->dependencyResolver()->resolve<IRpcService>();
			auto observable = rpcService->rpc(_loginRoute.c_str(), [authContext](bytestream* stream) {
				msgpack::pack(stream, authContext);
			}, PacketPriority::MEDIUM_PRIORITY);
			auto subscription = observable->subscribe([this, tce](Stormancer::Packetisp_ptr packet) {
				LoginResult loginResult(packet->stream);
				_client->getScene(loginResult.Token.c_str()).then([tce](Scene* scene) {
					tce.set(scene);
				});
			});
		});
		return pplx::create_task(tce);
	}

	pplx::task<Scene*> AuthenticationService::getAuthenticationScene()
	{
		if (!_authenticationSceneRetrieving)
		{
			_authenticationSceneRetrieving = true;
			pplx::task_completion_event<Scene*> tce;
			_client->getPublicScene(_authenticationSceneName.c_str(), nullptr).then([tce](Scene* scene) {
				scene->connect().then([tce, scene]() {
					tce.set(scene);
				});
			});
			_authenticationScene = pplx::create_task(tce);
		}
		return _authenticationScene;
	}

	pplx::task<void> AuthenticationService::logout()
	{
		pplx::task_completion_event<void> tce;
		if (_authenticated)
		{
			_authenticated = false;
			getAuthenticationScene().then([tce](Scene* scene) {
				scene->disconnect().then([scene, tce]() {
					scene->destroy();
					tce.set();
				});
			});
		}
		else
		{
			tce.set();
		}
		return pplx::create_task(tce);
	}
};

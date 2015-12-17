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

		pplx::task_completion_event<void> tce;
		auto a = pplx::create_task(tce);
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

	pplx::task<Result<Scene*>*> AuthenticationService::login(const char* pseudo, const char* password)
	{
		stringMap authContext{ { "provider", "loginpassword" }, { pseudo, password } };
		return login(&authContext);
	}

	pplx::task<Result<Scene*>*> AuthenticationService::steamLogin(const char* steamTicket)
	{
		stringMap authContext{ { "provider", "steam" }, { "ticket", steamTicket } };
		return login(&authContext);
	}

	pplx::task<Result<Scene*>*> AuthenticationService::login(const stringMap* authenticationContext)
	{
		pplx::task_completion_event<Result<Scene*>*> tce;
		auto result = new Result<Scene*>();
		try
		{
			auto authContext = processToDll(authenticationContext);

			if (_authenticated)
			{
				throw std::runtime_error("Authentication service already authenticated.");
			}

			getAuthenticationScene().then([this, authContext, tce, result](pplx::task<Scene*> t) {
				try
				{
					auto scene = t.get();
					auto rpcService = scene->dependencyResolver()->resolve<IRpcService>();
					auto observable = rpcService->rpc(_loginRoute.c_str(), [authContext](bytestream* stream) {
						msgpack::pack(stream, authContext);
					}, PacketPriority::MEDIUM_PRIORITY);
					auto onNext = [this, tce, result](Stormancer::Packetisp_ptr packet) {
						LoginResult loginResult(packet->stream);
						_client->getScene(loginResult.Token.c_str()).then([tce, result](Result<Scene*>* result2) {
							*result = *result2;
							tce.set(result);
							destroy(result2);
						});
					};
					auto onError = std::function<void(const char*)>([tce, result](const char* error) {
						result->setError(1, error);
						tce.set(result);
						ILogger::instance()->log(LogLevel::Error, "AuthenticationService", "onError", error);
					});
					auto onComplete = []() {};
					auto subscription = observable->subscribe(onNext, onError, onComplete);
				}
				catch (const std::exception& ex)
				{
					result->setError(1, ex.what());
					tce.set(result);
				}
			});
		}
		catch (const std::exception& ex)
		{
			result->setError(1, ex.what());
			tce.set(result);
		}
		return pplx::create_task(tce);
	}

	pplx::task<Scene*> AuthenticationService::getAuthenticationScene()
	{
		if (!_authenticationSceneRetrieving)
		{
			_authenticationSceneRetrieving = true;
			pplx::task_completion_event<Scene*> tce;
			_client->getPublicScene(_authenticationSceneName.c_str(), nullptr).then([tce](Result<Scene*>* result2) {
				if (result2->success())
				{
					auto scene = result2->get();
					scene->connect().then([tce, scene](Result<>* result) {
						if (result->success())
						{
							tce.set(scene);
						}
						else
						{
							tce.set_exception(std::runtime_error(result->reason()));
						}
						destroy(result);
					});
				}
				else
				{
					tce.set_exception(std::runtime_error(result2->reason()));
				}
				destroy(result2);
			});
			_authenticationScene = pplx::create_task(tce);
		}
		return _authenticationScene;
	}

	pplx::task<Result<>*> AuthenticationService::logout()
	{
		auto result = new Result<>();
		pplx::task_completion_event<Result<>*> tce;
		try
		{
			if (_authenticated)
			{
				_authenticated = false;
				getAuthenticationScene().then([tce, result](pplx::task<Scene*> t) {
					try
					{
						auto scene = t.get();
						scene->disconnect().then([scene, tce, result](Result<>* result2) {
							if (result2->success())
							{
								destroy(scene);
								result->set();
							}
							else
							{
								result->setError(1, result2->reason());
							}
							tce.set(result);
							destroy(result2);
						});
					}
					catch (const std::exception& ex)
					{
						result->setError(1, ex.what());
						tce.set(result);
					}
				});
			}
			else
			{
				result->set();
				tce.set(result);
			}
		}
		catch (const std::exception& ex)
		{
			result->setError(1, ex.what());
			tce.set(result);
		}
		return pplx::create_task(tce);
	}
};

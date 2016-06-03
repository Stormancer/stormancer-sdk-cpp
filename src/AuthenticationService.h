#pragma once
#include "headers.h"
#include "IAuthenticationService.h"

namespace Stormancer
{
	struct LoginResult
	{
	public:
		LoginResult(Stormancer::bytestream* stream);

	public:
		std::string errorMsg;
		bool success;
		std::string token;
		std::string userId;

	public:
		MSGPACK_DEFINE(errorMsg, success, token, userId);
	};

	class AuthenticationService : public IAuthenticationService
	{
	public:
		AuthenticationService(Client* client);
		virtual ~AuthenticationService();

		const char* authenticationSceneName();
		void setAuthenticationSceneName(const char* name);

		const char* createUserRoute();
		void setCreateUserRoute(const char* name);

		const char* loginRoute();
		void setLoginRoute(const char* name);

		pplx::task<Result<Scene*>*> login(const char* pseudo, const char* password);
		pplx::task<Result<Scene*>*> login(const stringMap* authenticationContext);
		pplx::task<Result<Scene*>*> getPrivateScene(const char* sceneId);
		pplx::task<Result<Scene*>*> steamLogin(const char* steamTicket);

		pplx::task<Scene*> getAuthenticationScene();

		pplx::task<Result<>*> logout();

		const char* userId();

	private:
		std::string _authenticationSceneName = "authenticator";
		std::string _createUserRoute = "provider.loginpassword.createAccount";
		std::string _loginRoute = "login";
		bool _authenticated = false;
		bool _authenticationInProgress = false;
		bool _authenticationSceneRetrieving = false;
		std::string _userId;

		Client* _client = nullptr;
		pplx::task<Scene*> _authenticationScene;
	};
};

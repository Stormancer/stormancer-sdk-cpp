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
		std::string ErrorMsg;
		bool Success = false;
		std::string Token;

	public:
		MSGPACK_DEFINE(ErrorMsg, Success, Token);
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

		pplx::task<Scene*> login(const char* pseudo, const char* password);
		pplx::task<Scene*> login(const stringMap* authenticationContext);
		pplx::task<Scene*> steamLogin(const char* steamTicket);

		pplx::task<Scene*> getAuthenticationScene();

		pplx::task<void> logout();

	private:
		std::string _authenticationSceneName = "authenticator";
		std::string _createUserRoute = "provider.loginpassword.createAccount";
		std::string _loginRoute = "login";
		bool _authenticated = false;
		bool _authenticationInProgress = false;
		bool _authenticationSceneRetrieving = false;

		Client* _client = nullptr;
		pplx::task<Scene*> _authenticationScene;
	};
};

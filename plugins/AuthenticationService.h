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
		std::string username;

	public:
		MSGPACK_DEFINE(errorMsg, success, token, userId, username);
	};
	struct CreateUserParameters
	{
	
		
	public:
		/// <summary>
		/// Player login
		/// </summary>
		std::string login;

		/// <summary>
		/// password stored in db (can/should be hash)
		/// </summary>
		std::string password;

		/// <summary>
		/// user email for password recovery 
		/// </summary>
		std::string email;

		/// <summary>
		/// Json user data
		/// </summary>
		std::string userData;

	public:
		MSGPACK_DEFINE(login, password, email, userData);
	};
	
	struct ChangePasswordParameters
	{
	public:
		std::string email;
		std::string code;
		std::string newPassword;
	public:
		MSGPACK_DEFINE(email, code, newPassword);
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

		pplx::task<Result<>*> createAccount(std::string login, std::string password, std::string email, std::string key, std::string pseudo);
		pplx::task<Result<Scene*>*> login(std::string email, std::string password);
		pplx::task<Result<Scene*>*> login(const stringMap authenticationContext);
		pplx::task<Result<Scene*>*> getPrivateScene(std::string sceneId);
		pplx::task<Result<Scene*>*> steamLogin(std::string steamTicket);

		pplx::task<std::shared_ptr<Result<void>>> requestPasswordChange(std::string email);
		pplx::task<std::shared_ptr<Result<void>>> changePassword(std::string email, std::string code, std::string newPassword);

		pplx::task<Scene*> getAuthenticationScene();

		pplx::task<Result<>*> logout();

		const char* userId();
		const char* GetUsername();

		GameConnectionState connectionState() const;
		Action<GameConnectionState>& connectionStateChangedAction();
		Action<GameConnectionState>::TIterator onConnectionStateChanged(std::function<void(GameConnectionState)> callback);

	private:
		void setConnectionState(GameConnectionState state);
	private:
		std::string _authenticationSceneName = "authenticator";
		std::string _createUserRoute = "provider.loginpassword.createAccount";
		std::string _loginRoute = "login";
		bool _authenticated = false;
		bool _authenticationInProgress = false;
		bool _authenticationSceneRetrieving = false;
		std::string _userId;
		std::string _username;

		Client* _client = nullptr;
		pplx::task<Scene*> _authenticationScene;

		Action<GameConnectionState> _onConnectionStateChanged;
		GameConnectionState _connectionState;
	};
};

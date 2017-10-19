#pragma once
#include <headers.h>
#include <Client.h>

namespace Stormancer
{
	enum class GameConnectionState
	{
		Disconnected = 0,
		Connecting = 1,
		Authenticated = 2,
		Disconnecting = 3,
		Authenticating = 4
	};

	struct LoginResult
	{
		std::string errorMsg;
		bool success;
		std::string userId;
		std::string username;

		MSGPACK_DEFINE(errorMsg, success, userId, username);
	};

	struct CreateUserParameters
	{
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

		MSGPACK_DEFINE(login, password, email, userData);
	};

	struct ChangePasswordParameters
	{
		std::string email;
		std::string code;
		std::string newPassword;

		MSGPACK_DEFINE(email, code, newPassword);
	};

	class AuthenticationService
	{
	public:

#pragma region public_methods

		AuthenticationService(Client* client);
		~AuthenticationService();

		std::string authenticationSceneName();
		void setAuthenticationSceneName(const std::string& name);

		std::string createUserRoute();
		void setCreateUserRoute(const std::string& name);

		std::string loginRoute();
		void setLoginRoute(const std::string& name);

		pplx::task<void> createAccount(const std::string& login, const std::string& password, const std::string& email, const std::string& key, const std::string& pseudo);
		pplx::task<void> login(const std::map<std::string, std::string>& authenticationContext);
		pplx::task<void> login(const std::string& email, const std::string& password);
		pplx::task<void> loginSteam(const std::string& ticket);







		pplx::task<Scene_ptr> getPrivateScene(const std::string& sceneId);

		//Impersonate an user using the impersonation plugin. The plugin should be disabled in production environments.
		pplx::task<void> impersonate(const std::string& provider, const std::string& claimPath, const std::string& uid, const std::string& impersonationSecret);

		pplx::task<void> requestPasswordChange(const std::string& email);
		pplx::task<void> changePassword(const std::string& email, const std::string& code, const std::string& newPassword);

		pplx::task<Scene_ptr> getAuthenticationScene();

		pplx::task<void> logout();

		std::string userId();
		std::string getUsername();

		pplx::task<std::string> getUserIdByPseudo(std::string pseudo);

		template<typename Iter>
		pplx::task<std::unordered_map<std::string, std::string>> getPseudos(Iter begin, Iter   end)
		{
			std::vector<std::string> input(begin, end);

			return getPseudos(input);
		}
		pplx::task<std::unordered_map<std::string, std::string>> getPseudos(std::vector<std::string> userIds);

		GameConnectionState connectionState() const;
		Action<GameConnectionState>& connectionStateChangedAction();
		Action<GameConnectionState>::TIterator onConnectionStateChanged(const std::function<void(GameConnectionState)>& callback);

#pragma endregion

	private:

#pragma region private_methods

		void setConnectionState(GameConnectionState state);

#pragma endregion

#pragma region private_members

		std::string _authenticationSceneName = "authenticator";
		std::string _createUserRoute = "provider.loginpassword.createAccount";
		std::string _loginRoute = "login";
		bool _authenticated = false;
		std::string _userId;
		std::string _username;
		Client* _client;
		pplx::task<Scene_ptr> _authenticationScene;
		Action<GameConnectionState> _onConnectionStateChanged;
		GameConnectionState _connectionState;
		rxcpp::composite_subscription _connectionSubscription;

#pragma endregion
	};
};

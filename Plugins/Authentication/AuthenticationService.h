#pragma once
#include "stormancer/headers.h"
#include "stormancer/Client.h"
#include "stormancer/Logger/ILogger.h"

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



	class AuthenticationService :std::enable_shared_from_this<AuthenticationService>
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

		pplx::task<void> login(const std::map<std::string, std::string>& authenticationContext);
		pplx::task<void> loginSteam(const std::string& ticket);







		pplx::task<Scene_ptr> connectToPrivateScene(const std::string& sceneId, std::function<void(Scene_ptr)> builder = [](Scene_ptr) {});

		//Impersonate an user using the impersonation plugin. The plugin should be disabled in production environments.
		pplx::task<void> impersonate(const std::string& provider, const std::string& claimPath, const std::string& uid, const std::string& impersonationSecret);


		pplx::task<Scene_ptr> getAuthenticationScene();

		pplx::task<void> logout();

		std::string userId();

		//get user id from bearer token
		pplx::task<std::string> getUserFromBearerToken(std::string token);

		//Creates a token authenticating the bearer as the userId
		pplx::task<std::string> getBearerToken();
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
		Action<GameConnectionState> _onConnectionStateChanged;
		GameConnectionState _connectionState;
		rxcpp::composite_subscription _connectionSubscription;
		ILogger_ptr _logger;

#pragma endregion
	};
};

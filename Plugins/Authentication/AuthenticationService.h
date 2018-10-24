#pragma once
#include "stormancer/headers.h"
#include "stormancer/Client.h"
#include "stormancer/Logger/ILogger.h"
#include "stormancer/Action2.h"
#include "stormancer/RPC/RpcService.h"
namespace Stormancer
{
	enum class GameConnectionState
	{
		Disconnected = 0,
		Connecting = 1,
		Authenticated = 2,
		Disconnecting = 3,
		Authenticating = 4,
		Reconnecting = 5
	};

	struct LoginResult
	{
		std::string errorMsg;
		bool success;
		std::string userId;
		std::string username;

		MSGPACK_DEFINE(errorMsg, success, userId, username);
	};

	struct OperationCtx
	{
		std::string operation;
		std::string originId;
		RpcRequestContext_ptr request;

	};


	class AuthenticationService :std::enable_shared_from_this<AuthenticationService>
	{
	public:

#pragma region public_methods

		AuthenticationService(Client* client);
		~AuthenticationService();


		pplx::task<void> login();
		pplx::task<void> logout();


		pplx::task<Scene_ptr> connectToPrivateScene(const std::string& sceneId, std::function<void(Scene_ptr)> builder = [](Scene_ptr) {});

		//Gets a connected scene for a service a serviceType and optional serviceName
		pplx::task<Scene_ptr> getSceneForService(const std::string& serviceType, const std::string& serviceName = "");

		pplx::task<Scene_ptr> getAuthenticationScene(pplx::cancellation_token ct = pplx::cancellation_token::none());

		std::string& userId();
		std::string& username();

		//get user id from bearer token
		pplx::task<std::string> getUserFromBearerToken(std::string token);

		//Creates a token authenticating the bearer as the userId
		pplx::task<std::string> getBearerToken();

		pplx::task<std::string> getUserIdByPseudo(std::string pseudo);


		GameConnectionState connectionState() const;
		Action2<GameConnectionState> connectionStateChanged;


		std::function<pplx::task<std::unordered_map<std::string, std::string>>()> getCredentialsCallback;

		template<typename TResult, typename... TArgs >
		pplx::task<TResult> sendRequestToUser(const std::string& userId, const std::string& operation, pplx::cancellation_token ct, const TArgs&... args)
		{
			return getAuthenticationScene().then([](Scene_ptr scene) {
				auto rpc = scene->dependencyResolver().lock()->resolve<RpcService>();
				return rpc->rpc("sendRequest", ct, args);
			});
			
		}

		void SetOperationHandler(std::string operation, std::function<pplx::task<void>(OperationCtx&)> handler);

#pragma endregion

	private:

#pragma region private_methods

		void setConnectionState(GameConnectionState state);
		pplx::task<Scene_ptr> loginImpl(int retry = 0);
		pplx::task<Scene_ptr> reconnect(int retry);
#pragma endregion

#pragma region private_members

		const std::string SCENE_ID = "authenticator";
		const std::string LOGIN_ROUTE = "login";

		bool _autoReconnect = true;

		std::string _userId;
		std::string _username;
		Client* _client;
		Action2<GameConnectionState> _connectionStateChanged;
		GameConnectionState _currentConnectionState;
		rxcpp::composite_subscription _connectionSubscription;
		ILogger_ptr _logger;

		//Task that completes when the user is authenticated.
		std::shared_ptr<pplx::task<Scene_ptr>> _authTask;

		std::unordered_map<std::string, std::function<pplx::task<void>(OperationCtx&)>> _operationHandlers;
#pragma endregion
	};
};

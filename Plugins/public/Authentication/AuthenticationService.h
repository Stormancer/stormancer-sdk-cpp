#pragma once
#include "stormancer/IClient.h"
#include "stormancer/Logger/ILogger.h"
#include "stormancer/Event.h"
#include "stormancer/RPC/Service.h"
#include "stormancer/Tasks.h"
#include "stormancer/msgpack_define.h"
#include <string>
#include <unordered_map>
#include <memory>
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunknown-pragmas"	// warning : unknown pragma ignored [-Wunknown-pragmas]
#endif

namespace Stormancer
{
	struct GameConnectionState
	{
		/// State of a network connection.
		enum State
		{
			Disconnected = 0,
			Connecting = 1,
			Authenticated = 2,
			Disconnecting = 3,
			Authenticating = 4,
			Reconnecting = 5
		};

		// Methods

		GameConnectionState() = default;
		GameConnectionState(State state2);
		GameConnectionState(State state2, std::string reason2);

		GameConnectionState& operator=(State state2);

		bool operator==(GameConnectionState& other) const;
		bool operator!=(GameConnectionState& other) const;

		bool operator==(State state2) const;
		bool operator!=(State state2) const;

		operator int();

		// Members

		State state = GameConnectionState::Disconnected;
		std::string reason;
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
	struct AuthParameters
	{
		std::string type;
		std::unordered_map<std::string, std::string> parameters;

		MSGPACK_DEFINE(type, parameters);
	};


	class AuthenticationService : public std::enable_shared_from_this<AuthenticationService>
	{
	public:

#pragma region public_methods

		AuthenticationService(std::shared_ptr<IClient> client);
		~AuthenticationService();


		pplx::task<void> login();
		pplx::task<void> logout();


		pplx::task<std::shared_ptr<Scene>> connectToPrivateScene(const std::string& sceneId, std::function<void(std::shared_ptr<Scene>)> builder = [](std::shared_ptr<Scene>) {});
		pplx::task<std::shared_ptr<Scene>> connectToPrivateSceneByToken(const std::string& token, std::function<void(std::shared_ptr<Scene>)> builder = [](std::shared_ptr<Scene>) {});
		//Gets a connected scene for a service a serviceType and optional serviceName
		pplx::task<std::shared_ptr<Scene>> getSceneForService(const std::string& serviceType, const std::string& serviceName = "", pplx::cancellation_token ct = pplx::cancellation_token::none());

		pplx::task<std::shared_ptr<Scene>> getAuthenticationScene(pplx::cancellation_token ct = pplx::cancellation_token::none());

		std::string& userId();
		std::string& username();

		//get user id from bearer token
		pplx::task<std::string> getUserFromBearerToken(std::string token);

		//Creates a token authenticating the bearer as the userId
		pplx::task<std::string> getBearerToken();

		pplx::task<std::string> getUserIdByPseudo(std::string pseudo);


		GameConnectionState connectionState() const;
		Event<GameConnectionState> connectionStateChanged;

		std::function<pplx::task<AuthParameters>()> getCredentialsCallback;

		template<typename TResult, typename... TArgs >
		pplx::task<TResult> sendRequestToUser(const std::string& userId, const std::string& operation, pplx::cancellation_token ct, const TArgs&... args)
		{
			return getAuthenticationScene().then([ct, userId, operation, args...](std::shared_ptr<Scene> scene) {
				auto rpc = scene->dependencyResolver().resolve<RpcService>();
				return rpc->rpc<TResult>("sendRequest", ct, userId, operation, args...);
			});

		}

		void SetOperationHandler(std::string operation, std::function<pplx::task<void>(OperationCtx&)> handler);

#pragma endregion

	private:

#pragma region private_methods

		void setConnectionState(GameConnectionState state);
		pplx::task<std::shared_ptr<Scene>> loginImpl(int retry = 0);
		pplx::task<std::shared_ptr<Scene>> reconnect(int retry);

#pragma endregion

#pragma region private_members

		const std::string SCENE_ID = "authenticator";
		const std::string LOGIN_ROUTE = "login";

		bool _autoReconnect = true;

		std::string _userId;
		std::string _username;
		std::weak_ptr<IClient> _client;
		Event<GameConnectionState> _connectionStateChanged;
		GameConnectionState _currentConnectionState;
		rxcpp::composite_subscription _connectionSubscription;
		ILogger_ptr _logger;

		//Task that completes when the user is authenticated.
		std::shared_ptr<pplx::task<std::shared_ptr<Scene>>> _authTask;

		std::unordered_map<std::string, std::function<pplx::task<void>(OperationCtx&)>> _operationHandlers;

#pragma endregion
	};
};

#ifdef __clang__
#pragma clang diagnostic pop
#endif
#pragma once

#include "stormancer/IClient.h"
#include "stormancer/Logger/ILogger.h"
#include "stormancer/Event.h"
#include "stormancer/RPC/Service.h"
#include "stormancer/Tasks.h"
#include "stormancer/msgpack_define.h"
#include "stormancer/DependencyInjection.h"
#include "stormancer/Utilities/TaskUtilities.h"
#include <string>
#include <unordered_map>
#include <memory>
#include <stdexcept>
#include <exception>
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunknown-pragmas"	// warning : unknown pragma ignored [-Wunknown-pragmas]
#endif


namespace Stormancer
{
	/// <summary>
	/// Manage user authentication and related functionality.
	/// </summary>
	/// This plugin is a header-only replacement for the Authentication plugin.
	/// <example>
	/// <code>
	/// auto conf = Stormancer::Configuration::create(...);
	/// ...
	/// conf->addPlugin(new Stormancer::Users::UsersPlugin);
	/// ...
	/// auto client = Stormancer::IClient::create(conf);
	/// ...
	/// auto users = client->dependencyResolver().resolve&lt;Stormancer::Users::UsersApi&gt;();
	/// users->login().wait();
	/// ...
	/// </code>
	/// </example>
	namespace Users
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
			GameConnectionState(State state2) : state(state2) {}
			GameConnectionState(State state2, std::string reason2) : state(state2), reason(reason2) {}

			GameConnectionState& operator=(State state2)
			{
				state = state2;
				return *this;
			}

			bool operator==(GameConnectionState& other) const
			{
				return state == other.state;
			}

			bool operator!=(GameConnectionState& other) const
			{
				return state != other.state;
			}

			bool operator==(State state2) const
			{
				return state == state2;
			}

			bool operator!=(State state2) const
			{
				return state != state2;
			}

			operator int()
			{
				return (int)state;
			}

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

		struct RenewCredentialsParameters
		{
			std::unordered_map<std::string, std::string> parameters;

			MSGPACK_DEFINE(parameters);
		};

		/// <summary>
		/// A platform-specific user Id.
		/// </summary>
		/// <remarks>
		/// For exmaple, it could be a Steam Id, a PSN Account Id or an Xbox User Id.
		/// This type is abstract. Only concrete subtypes implemented by platform support plugins (e.g Steam Plugin) can be instantiated.
		/// </remarks>
		struct PlatformUserId
		{
			/// <summary>
			/// This identifies the platform that this Id is for.
			/// </summary>
			/// <example>Steam, PSN...</example>
			/// <remarks>
			/// It should be unique across all subtypes of this class.
			/// This isn't actually enforced because it would be complicated with header-only plugins.
			/// </remarks>
			virtual std::string type() const = 0;

			/// <summary>
			/// This is the Id in string form.
			/// </summary>
			const std::string userId;

			virtual ~PlatformUserId() = default;

			bool operator==(const PlatformUserId& right)
			{
				return type() == right.type() && userId == right.userId;
			}

			bool operator!=(const PlatformUserId& right)
			{
				return !operator==(right);
			}

			std::string toString() const
			{
				return "[" + this->type() + "]" + userId;
			}

		protected:
			PlatformUserId(std::string id) : userId(id) {}
		};

		struct CredientialsContext
		{
			std::shared_ptr<AuthParameters> authParameters;
			std::shared_ptr<PlatformUserId> platformUserId;
		};

		class UsersApi;
		struct CredentialsRenewalContext
		{
			/// <summary>
			/// The type (name) of the provider that needs its credentials renewed
			/// </summary>
			std::string authProviderType;
			/// <summary>
			/// Parameters needed by the server-side authentication provider to renew the credentials. Must be set by the event handler.
			/// </summary>
			std::shared_ptr<RenewCredentialsParameters> response;
			std::shared_ptr<UsersApi> usersApi;
		};

		/// <summary>
		/// Run custom code to provide or modify authentication credentials.
		/// </summary>
		/// <remarks>
		/// This interface allows injecting custom logic into the authentication process.
		/// When the client needs to authenticate with the Stormancer application, it has to provide credentials.
		/// The nature of these credentials depends on the platform that the client is running on (PC with Steam or another platform, consoles...),
		/// as well as possibly custom logic on the server application.
		/// This means that the logic needed to retrieve these credentials is at least platform-specific, and maybe even game-specific for more complex scenarios.
		/// In order to provide this logic, at least one plugin that provides a class implementing <c>IAuthenticationEventHandler</c> must be registered in the client.
		/// Typically, you should register the one that corresponds to your platform (e.g SteamPlugin, PSNPlugin, XboxLivePlugin...).
		/// If you need additional authentication parameters for your game, you would create a custom plugin with a class that implements <c>IAuthenticationEventHandler</c>,
		/// then in your custom <c>IPlugin</c> class, override <c>IPlugin::registerClientDependencies()</c>, and inside this method,
		/// register your custom <c>IAuthenticationEventHandler</c> in the <c>ContainerBuilder</c>.
		/// </remarks>
		class IAuthenticationEventHandler
		{
		public:

			/// <summary>
			/// Add or update credentials.
			/// </summary>
			/// <remarks>
			/// Add the elements required by your server-side authentication logic inside <c>context.authParameters</c>.
			/// There can be multiple <c>IAuthenticationEventHandler</c> instances registered at once ;
			/// each of their <c>retrieveCredentials()</c> method will be run sequentially, in an undefined order.
			/// </remarks>
			/// <param name="context">
			/// An object that holds <c>AuthParameters</c> for the current authentication request.
			/// Provide the necessary credentials for your application by setting <c>context.authParameters</c>.
			/// </param>
			/// <returns>
			/// A pplx::task&lt;void&gt; that should complete when the processing that you needed to do is done.
			/// You must not modify <c>context</c> after this task has completed, or else you would run into a race condition.
			/// </returns>
			virtual pplx::task<void> retrieveCredentials(const CredientialsContext& context) = 0;

			/// <summary>
			/// Fulfill a request from the server to renew credentials for a specific authentication provider.
			/// </summary>
			/// <remarks>
			/// You should override this method if your server-side authentication logic requires renewal of client credentials.
			/// In this case, this method will be called when such a renewal is requested by the server.
			/// </remarks>
			/// <remarks>
			/// Credentials renewal is performed for a single authentication provider at a time.
			/// The type of the provider that requeted the renewal can be obtained from <paramref name="context"></paramref>.
			/// </remarks>
			/// <param name="context">
			/// Object containing information about the current renewal request.
			/// The parameters needed by the authentication provider to perform the renewal must be set by the handler in <c>context.response->parameters</c>.
			/// </param>
			/// <returns>
			/// A pplx::task&lt;void&gt; that should complete when the processing that you needed to do is done.
			/// You must not modify <c>context</c> after this task has completed, or else you would run into a race condition.
			/// </returns>
			virtual pplx::task<void> renewCredentials(const CredentialsRenewalContext& /*context*/)
			{
				return pplx::task_from_result();
			}

			virtual ~IAuthenticationEventHandler() = default;
		};

		/// <summary>
		/// An exception denoting an error in retrieving user credentials from an <c>IAuthenticationEventHandler</c> instance.
		/// </summary>
		class CredentialsException : public std::exception
		{
		public:
			CredentialsException(const char* message, const std::exception& innerException) : message(makeMessage(message, innerException)) {}

			const char* what() const noexcept override
			{
				return message.c_str();
			}

		private:
			static std::string makeMessage(const char* message, const std::exception& innerException)
			{
				return std::string(message) + " [Inner exception message: " + std::string(innerException.what()) + "]";
			}

			std::string message;
		};

		/// <summary>
		/// The main class to leverage Users functionality.
		/// </summary>
		class UsersApi : public std::enable_shared_from_this<UsersApi>
		{
		public:

#pragma region public_methods

			UsersApi(
				std::shared_ptr<IClient> client,
				std::vector<std::shared_ptr<IAuthenticationEventHandler>> authEventHandlers,
				std::shared_ptr<IActionDispatcher> userDispatcher
			)
				: _client(client)
				, _logger(client->dependencyResolver().resolve<ILogger>())
				, _authenticationEventHandlers(authEventHandlers)
				, _userDispatcher(userDispatcher)
			{
			}

			~UsersApi()
			{
				_connectionSubscription.unsubscribe();
			}

			/// <summary>
			/// Set the platform-specific user that should be authenticated with Stormancer.
			/// </summary>
			/// <remarks>
			/// When using Stormancer in tandem with one or more online platforms such as Steam or the PSN,
			/// your game has to provide Stormancer with the Id of the current user.
			/// This is what this method is here for.
			/// If you do use such a platform, you should call this method before calling <c>login()</c>,
			/// and every time the in-game user changes. In the latter case, this method will disconnect the previous user and authenticate the new one.
			/// If you do not use the platform-specific functionality of Stormancer, you do not need to call this method.
			/// </remarks>
			/// <param name="userId">
			/// The platform-specific Id of the user.
			/// You should instantiate it using the factory method provided by the platform-specific plugin of your choice.
			/// </param>
			/// <returns>
			/// A <c>pplx::task</c> that completes when the user change operation is done.
			/// - If no user was logged in, it will complete immediately. You can then call <c>login()</c> to perform the authentication.
			/// - If a different user was already logged in, it will complete when the previous user has been logged out and the new one (<c>userId</c>) has been authenticated.
			/// </returns>
			pplx::task<void> setCurrentLocalUser(std::shared_ptr<PlatformUserId> userId)
			{
				if (userId == nullptr)
				{
					_currentLocalUser = nullptr;
					return logout();
				}

				if (_currentConnectionState == GameConnectionState::Disconnected || _currentConnectionState == GameConnectionState::Disconnecting)
				{
					_currentLocalUser = userId;
					return pplx::task_from_result();
				}

				if (_currentLocalUser == nullptr || *_currentLocalUser != *userId)
				{
					_currentLocalUser = userId;
					std::weak_ptr<UsersApi> weakThis = this->shared_from_this();
					return logout().then([weakThis]
					{
						if (auto that = weakThis.lock())
						{
							return that->login();
						}
						return pplx::task_from_result();
					});
				}

				// Last case: connected with _currentLocalUser == userId - do nothing
				return pplx::task_from_result();
			}

			/// <summary>
			/// Retrieve the current local user, as set by <c>setCurrentLocalUser()</c>.
			/// </summary>
			/// <remarks>
			/// This will return an empty <c>std::shared_ptr</c> if no user has been set.
			/// </remarks>
			/// <returns>The platform-specific Id of the current user.</returns>
			/// <seealso cref="setCurrentLocalUser()"/>
			std::shared_ptr<PlatformUserId> getCurrentLocalUser() const { return _currentLocalUser; }

			/// <summary>
			/// Authenticate with the Stormancer server application.
			/// </summary>
			/// <remarks>
			/// Authentication is required to access private scenes on the server application.
			/// </remarks>
			/// <returns>
			/// A <c>pplx::task</c> that completes when the authentication is done.
			/// If the authentication fails, the task will be faulted.
			/// In case the authentication process fails when retrieving local credentals, the type of the exception embedded in the task will be <c>CredentialsException</c>.
			/// </returns>
			pplx::task<void> login()
			{
				_autoReconnect = true;
				return getAuthenticationScene().then([](std::shared_ptr<Scene>) {});
			}

			/// <summary>
			/// Log out of Stormancer.
			/// </summary>
			/// <remarks>
			/// This will trigger a disconnection from every scene.
			/// </remarks>
			/// <returns>A <c>pplx::task</c> that completes when the disconnection process is done.</returns>
			pplx::task<void> logout()
			{
				_autoReconnect = false;
				if (_currentConnectionState != GameConnectionState::Disconnected && _currentConnectionState != GameConnectionState::Disconnecting)
				{
					this->setConnectionState(GameConnectionState::Disconnecting);

					return getAuthenticationScene()
						.then([](std::shared_ptr<Scene> scene)
					{
						return scene->disconnect();
					})
						.then([](auto t)
					{
						try
						{
							t.get();
						}
						catch (std::exception&)
						{
						}
					});
				}
				else
				{
					return pplx::task_from_result();
				}
			}

			pplx::task<std::string> getSceneConnectionToken(const std::string& serviceType, const std::string& serviceName, pplx::cancellation_token ct)
			{
				auto logger = this->_logger;
				return getAuthenticationScene(ct)
					.then([serviceType, serviceName, ct, logger](std::shared_ptr<Scene> authScene)
				{

					auto rpcService = authScene->dependencyResolver().resolve<RpcService>();
					logger->log(LogLevel::Info, "authentication", "Getting token for " + serviceType + " and name  " + serviceName);
					return rpcService->rpc<std::string>("Locator.GetSceneConnectionToken", ct, serviceType, serviceName).then([logger, serviceType, serviceName](pplx::task<std::string> t) {

						try
						{
							auto token = t.get();
							logger->log(LogLevel::Info, "authentication", "Got token for " + serviceType + " and name  " + serviceName);
							return token;
						}
						catch (std::exception& ex)
						{
							logger->log(LogLevel::Error, "authentication", "Failed getting token for " + serviceType + " and name  " + serviceName, ex.what());
							throw;
						}

					});
				});
			}

			pplx::task<std::shared_ptr<Scene>> connectToPrivateScene(const std::string& sceneId, std::function<void(std::shared_ptr<Scene>)> builder = [](std::shared_ptr<Scene>) {})
			{
				std::weak_ptr<UsersApi> wThat = this->shared_from_this();
				return getAuthenticationScene()
					.then([sceneId](std::shared_ptr<Scene> authScene)
				{
					auto rpcService = authScene->dependencyResolver().resolve<RpcService>();
					return rpcService->rpc<std::string, std::string>("sceneauthorization.gettoken", sceneId);
				})
					.then([wThat, builder](std::string token)
				{
					auto that = wThat.lock();

					if (that)
					{
						if (auto client = that->_client.lock())
						{
							return client->connectToPrivateScene(token, builder);
						}
					}

					throw std::runtime_error("Client is invalid.");
				});
			}

			pplx::task<std::shared_ptr<Scene>> connectToPrivateSceneByToken(const std::string& token, std::function<void(std::shared_ptr<Scene>)> builder = [](std::shared_ptr<Scene>) {})
			{
				std::weak_ptr<UsersApi> wThat = this->shared_from_this();
				return getAuthenticationScene()
					.then([token, wThat, builder](std::shared_ptr<Scene> authScene)
				{
					auto that = wThat.lock();

					if (that)
					{
						if (auto client = that->_client.lock())
						{
							return client->connectToPrivateScene(token, builder);
						}
					}

					throw std::runtime_error("Client is invalid.");
				});
			}

			/// <summary>
			/// Get a connected scene for a service.
			/// </summary>
			/// <param name="serviceType">The type of the service</param>
			/// <param name="serviceName">The name of the service (optional)</param>
			/// <param name="ct">Optional cancellation token, if you want the ability to cancel the underlying request</param>
			/// <returns>A <c>pplx::task</c> that completes when the scene has been retrieved.</returns>
			pplx::task<std::shared_ptr<Scene>> getSceneForService(const std::string& serviceType, const std::string& serviceName = "", pplx::cancellation_token ct = pplx::cancellation_token::none())
			{
				std::weak_ptr<UsersApi> wThat = this->shared_from_this();

				return getSceneConnectionToken(serviceType, serviceName, ct)
					.then([wThat, ct, serviceType, serviceName](pplx::task<std::string> task)
				{
					try
					{
						auto token = task.get();
						auto that = wThat.lock();

						if (that)
						{
							that->_logger->log(LogLevel::Info, "authentication", "Retrieved scene connection token for service type " + serviceType + " and name  " + serviceName);

							if (auto client = that->_client.lock())
							{
								return client->connectToPrivateScene(token, Stormancer::IClient::SceneInitializer(), ct);
							}
						}

						throw std::runtime_error("Client is invalid.");
					}
					catch (std::exception& ex)
					{
						if (auto that = wThat.lock())
						{
							that->_logger->log(LogLevel::Error, "authentication", "Failed to get scene connection token for service type " + serviceType + " and name " + serviceName, ex.what());
						}
						throw;
					}
				});
			}

			pplx::task<std::shared_ptr<Scene>> getAuthenticationScene(pplx::cancellation_token ct = pplx::cancellation_token::none())
			{

				if (_client.expired())
				{
					return pplx::task_from_exception<std::shared_ptr<Scene>>(std::runtime_error("Client is invalid."));
				}

				if (!_authTask)
				{
					if (!_autoReconnect)
					{
						return pplx::task_from_exception<std::shared_ptr<Scene>>(std::runtime_error("Authenticator disconnected. Call login before using the UsersApi."));
					}
					else
					{
						_authTask = std::make_shared<pplx::task<std::shared_ptr<Scene>>>(loginImpl());
					}
				}

				auto t = *_authTask;
				std::weak_ptr<UsersApi> wThat = this->shared_from_this();
				pplx::task_completion_event<std::shared_ptr<Scene>> tce;
				if (ct.is_cancelable())
				{
					ct.register_callback([tce]()
					{
						tce.set_exception(pplx::task_canceled());
					});
				}
				t.then([tce, wThat](pplx::task<std::shared_ptr<Scene>> t)
				{
					try
					{
						auto scene = t.get();
						if (scene)
						{
							tce.set(scene);
						}
						else
						{
							throw std::runtime_error("Authentication failed");
						}
					}
					catch (...)
					{
						if (auto that = wThat.lock())
						{
							that->_authTask = nullptr;
						}
						tce.set_exception(std::current_exception());
					}
				});

				return pplx::create_task(tce, _userDispatcher);
			}

			const std::string& userId() const
			{
				return _userId;
			}

			const std::string& username() const
			{
				return _username;
			}

			/// <summary>
			/// Get a user's id from a bearer token.
			/// </summary>
			/// <param name="token">A bearer token, sent to you by another user.</param>
			/// <returns>
			/// A <c>pplx::task</c> that completes when the server has verified the token.
			/// If the token is valid, its result will be the Id of the user who made the bearer token request
			/// </returns>
			pplx::task<std::string> getUserFromBearerToken(std::string token)
			{
				return getAuthenticationScene()
					.then([token](std::shared_ptr<Scene> authScene)
				{
					auto rpcService = authScene->dependencyResolver().resolve<RpcService>();
					return rpcService->rpc<std::string, std::string>("sceneauthorization.getuserfrombearertoken", token);
				});
			}

			/// <summary>
			/// Create a bearer token that can be used to authenticate the current user.
			/// </summary>
			/// <remarks>
			/// 
			/// </remarks>
			/// <returns>A <c>pplx::task</c> that completes when the bearer token has been created. The result of the task is the bearer token.</returns>
			pplx::task<std::string> getBearerToken()
			{
				return getAuthenticationScene()
					.then([](std::shared_ptr<Scene> authScene)
				{
					auto rpcService = authScene->dependencyResolver().resolve<RpcService>();
					return rpcService->rpc<std::string>("sceneauthorization.getbearertoken");
				});
			}

			pplx::task<std::string> getUserIdByPseudo(std::string pseudo)
			{
				return getAuthenticationScene()
					.then([pseudo](std::shared_ptr<Scene> authScene)
				{
					auto rpcService = authScene->dependencyResolver().resolve<RpcService>();
					return rpcService->rpc<std::string, std::string>("users.getuseridbypseudo", pseudo);
				});
			}

			GameConnectionState connectionState() const
			{
				return _currentConnectionState;
			}

			Event<GameConnectionState> connectionStateChanged;

			/// \deprecated Use <c>IAuthenticationEventHandler</c> instead.
			std::function<pplx::task<AuthParameters>()> getCredentialsCallback;

			//Returns the current authentication status of the user: The list of authentication providers that successfully 
			pplx::task<std::unordered_map<std::string, std::string>> getAuthenticationStatus(pplx::cancellation_token ct = pplx::cancellation_token::none())
			{
				return getAuthenticationScene().then([ct](std::shared_ptr<Scene> scene) {

					auto rpc = scene->dependencyResolver().resolve<RpcService>();
					return rpc->rpc<std::unordered_map<std::string, std::string>>("Authentication.GetStatus", ct);
				});
			}

			//Get the metadata for the authentication system, advertising what kind of authentication is available and which parameters it supports.
			pplx::task<std::unordered_map<std::string, std::string>> getMetadata(pplx::cancellation_token ct = pplx::cancellation_token::none())
			{
				return getAuthenticationScene().then([ct](std::shared_ptr<Scene> scene) {

					auto rpc = scene->dependencyResolver().resolve<RpcService>();
					return rpc->rpc<std::unordered_map<std::string, std::string>>("Authentication.GetMetadata", ct);
				});
			}

			//Perform an additionnal authentication on an already connected client (for instance to perform linking)
			pplx::task<std::unordered_map<std::string, std::string>> authenticate(AuthParameters p, pplx::cancellation_token ct = pplx::cancellation_token::none())
			{
				std::weak_ptr<UsersApi> wThat = this->shared_from_this();
				return getAuthenticationScene().then([ct, p](std::shared_ptr<Scene> scene) {


					auto rpc = scene->dependencyResolver().resolve<RpcService>();
					return rpc->rpc<LoginResult>("Authentication.Login", ct, p);

				}).then([ct, wThat](LoginResult r) {

					auto that = wThat.lock();
					if (!that)
					{
						throw PointerDeletedException("client destroyed");
					}

					return that->getAuthenticationStatus(ct);
				});
			}

			//Setups an authentication provider
			pplx::task<void> setup(AuthParameters p, pplx::cancellation_token ct = pplx::cancellation_token::none())
			{
				return getAuthenticationScene().then([p, ct](std::shared_ptr<Scene> scene) {

					auto rpc = scene->dependencyResolver().resolve<RpcService>();
					return rpc->rpc<void>("Authentication.Register", ct, p);

				});
			}

			//Unlink the authenticated user from auth provided by the specified provider
			pplx::task<void> unlink(std::string type, pplx::cancellation_token ct = pplx::cancellation_token::none())
			{
				return getAuthenticationScene().then([ct, type](std::shared_ptr<Scene> scene) {

					auto rpc = scene->dependencyResolver().resolve<RpcService>();
					return rpc->rpc<void>("Authentication.Unlink", ct, type);

				});
			}

			template<typename TResult, typename... TArgs >
			pplx::task<TResult> sendRequestToUser(const std::string& userId, const std::string& operation, pplx::cancellation_token ct, const TArgs&... args)
			{
				return getAuthenticationScene().then([ct, userId, operation, args...](std::shared_ptr<Scene> scene)
				{
					auto rpc = scene->dependencyResolver().resolve<RpcService>();
					return rpc->rpc<TResult>("sendRequest", ct, userId, operation, args...);
				});
			}

			void setOperationHandler(std::string operation, std::function<pplx::task<void>(OperationCtx&)> handler)
			{
				_operationHandlers[operation] = handler;
			}

			pplx::task<void> registerNewUser(std::string type, std::unordered_map<std::string, std::string> data)
			{
				auto ctx = AuthParameters();
				ctx.type = type;
				ctx.parameters = data;

				return getAuthenticationScene()
					.then([ctx](std::shared_ptr<Scene> scene)
				{
					auto rpcService = scene->dependencyResolver().resolve<RpcService>();
					return rpcService->rpc<void>("Authentication.Register", ctx);
				});
			}

#pragma endregion

		private:

			static constexpr int RETRY_COUNTER_MAX = 100;

#pragma region private_methods

			void setConnectionState(GameConnectionState state)
			{
				if (_currentConnectionState != state)
				{
					std::string reason = state.reason.empty() ? "" : ", reason : " + state.reason;
					this->_logger->log(LogLevel::Info, "connection", "Game connection state changed", std::to_string((int)state) + reason);

					if (state == GameConnectionState::Disconnected)
					{
						_authTask = nullptr;
						if (state.reason == "User connected elsewhere" || state.reason == "Authentication failed" || state.reason == "auth.login.new_connection")
						{
							_autoReconnect = false;
							if (auto client = _client.lock())
							{
								client->disconnect();//Disconnect still connected scenes.
							}
						}
						if (_autoReconnect && !_client.expired())
						{
							setConnectionState(GameConnectionState::Reconnecting);
						}
						else
						{
							_currentConnectionState = state;
							connectionStateChanged(state);
						}
					}
					else if (state == GameConnectionState::Reconnecting && _currentConnectionState != GameConnectionState::Reconnecting)
					{
						_currentConnectionState = state;
						connectionStateChanged(state);
						auto logger = _logger;
						this->getAuthenticationScene()
							.then([logger](pplx::task<std::shared_ptr<Scene>> t)
						{
							try
							{
								t.get();
							}
							catch (const std::exception& ex)
							{
								logger->log(LogLevel::Error, "connection", "Reconnection failed due to an unrecoverable error", ex);
							}
						});
					}
					else
					{
						_currentConnectionState = state;
						connectionStateChanged(state);
					}
				}
			}

			pplx::task<std::shared_ptr<Scene>> loginImpl(int retry = 0)
			{
				setConnectionState(GameConnectionState::Connecting);
				std::weak_ptr<UsersApi> wThat = this->shared_from_this();

				if (_authenticationEventHandlers.empty() && !this->getCredentialsCallback)
				{
					_autoReconnect = false;
					setConnectionState(GameConnectionState::Disconnected);
					return pplx::task_from_exception<std::shared_ptr<Scene>>(std::runtime_error("No IAuthenticationEventHandler are present, and 'getCredentialsCallback' is not set. At least one IAuthenticationEventHandler should be available in the client's DependencyScope, or 'getCredentialsCallback' should be set."));
				}
				auto client = _client.lock();
				if (!client)
				{
					_autoReconnect = false;
					setConnectionState(GameConnectionState::Disconnected);
					return pplx::task_from_exception<std::shared_ptr<Scene>>(std::runtime_error("Client destroyed."));
				}

				return client->connectToPublicScene(SCENE_ID, [wThat](std::shared_ptr<Scene> scene)
				{
					auto that = wThat.lock();
					if (that)
					{
						that->_connectionSubscription = scene->getConnectionStateChangedObservable().subscribe([wThat](ConnectionState state)
						{
							auto that = wThat.lock();
							if (that)
							{
								switch (state)
								{
								case ConnectionState::Disconnecting:
									that->setConnectionState(GameConnectionState::Disconnecting);
									break;
								case ConnectionState::Disconnected:
									that->setConnectionState(GameConnectionState(GameConnectionState::State::Disconnected, state.reason));
									break;
								case ConnectionState::Connecting:
									that->connectionStateChanged(GameConnectionState::Connecting);
									break;
								default:
									break;
								}
							}
						});
					}

					auto rpcService = scene->dependencyResolver().resolve<RpcService>();

					rpcService->addProcedure("sendRequest", [wThat](RpcRequestContext_ptr ctx)
					{
						OperationCtx opCtx;
						opCtx.request = ctx;
						Serializer serializer;
						serializer.deserialize(ctx->inputStream(), opCtx.originId, opCtx.operation);

						auto that = wThat.lock();
						if (!that)
						{
							throw (std::runtime_error("UsersApi destroyed"));
						}

						auto it = that->_operationHandlers.find(opCtx.operation);
						if (it == that->_operationHandlers.end())
						{
							throw (std::runtime_error("operation.notfound"));
						}

						return it->second(opCtx);
					});

					rpcService->addProcedure("users.renewCredentials", [wThat](RpcRequestContext_ptr ctx)
					{
						auto that = wThat.lock();
						if (!that)
						{
							return pplx::task_from_result();
						}

						auto provider = ctx->readObject<std::string>();
						that->_logger->log(LogLevel::Trace, "UsersApi", "Received a renewCredentials request for provider " + provider);

						auto logger = that->_logger;
						return that->runCredentialsRenewalHandlers(provider)
							.then([ctx, logger](pplx::task<RenewCredentialsParameters> task)
						{
							try
							{
								ctx->sendValueTemplated(task.get());
							}
							catch (const std::exception& ex)
							{
								logger->log(LogLevel::Error, "UsersApi", "An exception was thrown by a renewCredentials handler", ex);
							}
						});
					});
				})
					.then([wThat](std::shared_ptr<Scene> scene)
				{
					auto that = wThat.lock();

					if (!that)
					{
						throw (std::runtime_error("UsersApi destroyed"));
					}

					if (!that->_autoReconnect)
					{
						throw std::runtime_error("Auto recconnection is disable please login before");
					}

					return that->runCredentialsEventHandlers()
						.then([scene, wThat](pplx::task<AuthParameters> ctxTask)
					{
						AuthParameters ctx;
						auto that = wThat.lock();
						try
						{
							ctx = ctxTask.get();
						}
						catch (const std::exception& ex)
						{
							// if an exception was thrown by auth event handlers, do not try to reconnect
							if (that)
							{
								that->_autoReconnect = false;
							}
							throw CredentialsException("An exception was thrown by an IAuthenticationEventHandler::retrieveCredentials() call", ex);
						}
						if (!that)
						{
							throw std::runtime_error("destroyed");
						}
						auto rpcService = scene->dependencyResolver().resolve<RpcService>();
						return rpcService->rpc<LoginResult>("Authentication.Login", ctx);
					})
						.then([scene, wThat](LoginResult result)
					{
						auto that = wThat.lock();
						if (!that)
						{
							throw std::runtime_error("destroyed");
						}

						if (!result.success)
						{
							that->_autoReconnect = false;//disable auto reconnection
							that->setConnectionState(GameConnectionState::Disconnected);
							throw std::runtime_error("Login failed : " + result.errorMsg);
						}
						else
						{
							that->_userId = result.userId;
							that->_username = result.username;
							that->setConnectionState(GameConnectionState::Authenticated);
						}

						return scene;
					});

				}, _userDispatcher)
					.then([wThat, retry](pplx::task<std::shared_ptr<Scene>> t)
				{
					try
					{
						return pplx::task_from_result(t.get());
					}
					catch (const std::exception& ex)
					{
						auto that = wThat.lock();
						if (that && that->_autoReconnect && that->connectionState() != GameConnectionState::Disconnected)
						{
							that->_logger->log(LogLevel::Warn, "UsersApi::loginImpl", "Login failed with recoverable error, doing another attempt", ex);
							return that->reconnect(std::min(retry + 1, int(RETRY_COUNTER_MAX))); //temp value to work around compiler limitation with constexpr
						}
						else
						{
							throw;
						}
					}
				});
			}

			pplx::task<std::shared_ptr<Scene>> reconnect(int retry)
			{
				using namespace std::chrono_literals;
				auto delay = retry * 1000ms;
				if (delay > 5000ms)
				{
					delay = 5000ms;
				}

				std::weak_ptr<UsersApi> wThat = this->shared_from_this();

				this->setConnectionState(GameConnectionState::Reconnecting);
				return taskDelay(delay)
					.then([wThat, retry]()
				{
					auto that = wThat.lock();
					if (!that)
					{
						throw std::runtime_error("destroyed");
					}
					return that->loginImpl(retry);
				});
			}

			pplx::task<AuthParameters> runCredentialsEventHandlers()
			{
				pplx::task<AuthParameters> getCredsTask = pplx::task_from_result<AuthParameters>(AuthParameters());

				if (getCredentialsCallback)
				{
					getCredsTask = getCredentialsCallback();
				}

				std::weak_ptr<UsersApi> weakThis = this->shared_from_this();
				return getCredsTask.then([weakThis](AuthParameters authParameters)
				{
					auto that = weakThis.lock();
					if (!that)
					{
						throw std::runtime_error("UsersApi destroyed");
					}

					CredientialsContext credentialsContext;
					credentialsContext.authParameters = std::make_shared<AuthParameters>(authParameters);
					credentialsContext.platformUserId = that->_currentLocalUser;
					pplx::task<void> eventHandlersTask = pplx::task_from_result();
					for (auto evHandler : that->_authenticationEventHandlers)
					{
						eventHandlersTask = eventHandlersTask.then([evHandler, credentialsContext]
						{
							return evHandler->retrieveCredentials(credentialsContext);
						}, that->_userDispatcher);
					}
					return eventHandlersTask.then([credentialsContext]
					{
						return *credentialsContext.authParameters;
					});
				});
			}

			pplx::task<RenewCredentialsParameters> runCredentialsRenewalHandlers(const std::string& providerType)
			{
				CredentialsRenewalContext context;
				context.authProviderType = providerType;
				context.response = std::make_shared<RenewCredentialsParameters>();
				context.usersApi = this->shared_from_this();

				pplx::task<void> handlersTask = pplx::task_from_result();
				for (const auto& handler : _authenticationEventHandlers)
				{
					handlersTask = handlersTask.then([handler, context]
					{
						return handler->renewCredentials(context);
					}, _userDispatcher);
				}

				return handlersTask.then([context]
				{
					return *context.response;
				});
			}

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
			std::vector<std::shared_ptr<IAuthenticationEventHandler>> _authenticationEventHandlers;
			std::shared_ptr<IActionDispatcher> _userDispatcher;
			// The current platform-specific local user, set by the game using setCurrentLocalUser().
			std::shared_ptr<PlatformUserId> _currentLocalUser;

#pragma endregion
		};

		class UsersPlugin : public IPlugin
		{
		public:
			void registerClientDependencies(ContainerBuilder& builder) override
			{
				builder.registerDependency<UsersApi,
					IClient,
					ContainerBuilder::All<IAuthenticationEventHandler>,
					IActionDispatcher
				>().singleInstance();
			}

			void clientDisconnecting(std::shared_ptr<IClient> client) override
			{
				auto user = client->dependencyResolver().resolve<UsersApi>();
				user->logout();
			}
		};

	}
}

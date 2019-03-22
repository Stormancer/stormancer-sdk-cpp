#pragma once

#include "stormancer/BuildConfig.h"

#include "stormancer/Tasks.h"
#include "stormancer/Scene.h"
#include "stormancer/Configuration.h"
#include "stormancer/Federation.h"
namespace Stormancer
{
	class IClient
	{
	public:
		using SceneInitializer = std::function<void(std::shared_ptr<Scene>)>;

		/// <summary>
		/// Factory method that constructs an <c>IClient</c> from a <c>Configuration</c>.
		/// </summary>
		/// <param name="config"><c>Configuration</c> object containing parameters for the client to be created</param>
		/// <returns></returns>
		static std::shared_ptr<IClient> create(Configuration_ptr config);

		/// <summary>
		/// Connect to an existing public scene on the server application.
		/// </summary>
		/// <param name="sceneId">Id (name) of the scene</param>
		/// <param name="initializer">Callback used to register route handlers on the scene</param>
		/// <param name="ct">Token that can be used to cancel the connection while it is still in progress</param>
		/// <returns>A <c>pplx::task</c> that completes when the connection to the scene is established</returns>
		virtual pplx::task<std::shared_ptr<Scene>> connectToPublicScene(const std::string& sceneId, const SceneInitializer& initializer = SceneInitializer(), pplx::cancellation_token ct = pplx::cancellation_token::none()) = 0;

		/// <summary>
		/// Connect to an existing private scene on the server application.
		/// </summary>
		/// <param name="sceneToken">Authentication token needed to connect to the private scene</param>
		/// <param name="sceneId">Id (name) of the scene</param>
		/// <param name="initializer">Callback used to register route handlers on the scene</param>
		/// <param name="ct">Token that can be used to cancel the connection while it is still in progress</param>
		/// <returns>A <c>pplx::task</c> that completes when the connection to the scene is established</returns>
		virtual pplx::task<std::shared_ptr<Scene>> connectToPrivateScene(const std::string& sceneToken, const SceneInitializer& initializer = SceneInitializer(), pplx::cancellation_token ct = pplx::cancellation_token::none()) = 0;

		/// <summary>
		/// Disconnect from the server application and from every scene this client is connected to.
		/// </summary>
		/// <returns></returns>
		virtual pplx::task<void> disconnect() = 0;

		/// <summary>
		/// Get sync clock value.
		/// </summary>
		virtual int64 clock() const = 0;

		/// <summary>
		/// Get last ping from sync clock.
		/// </summary>
		virtual int64 lastPing() const = 0;

		/// <summary>
		/// Get the client's dependency resolver.
		/// </summary>
		/// <returns></returns>
		virtual std::shared_ptr<DependencyResolver> dependencyResolver() = 0;



		/// <summary>
		/// Set connection metadata.
		/// </summary>
		/// <param name="key">Metadata key</param>
		/// <param name="value">Matadata value</param>
		virtual void setMedatata(const std::string& key, const std::string& value) = 0;

		/// <summary>
		/// Get the ping to a cluster in the federation.
		/// </summary>
		/// <param name="clusterId">Id of the cluster to ping</param>
		/// <param name="ct">Token to cancel the ping request</param>
		/// <returns>A <c>pplx::task</c> containing the ping value in milliseconds, that completes when the ping operation finishes.</returns>
		virtual pplx::task<int> pingCluster(std::string clusterId, pplx::cancellation_token ct = pplx::cancellation_token::none()) = 0;

		/// <summary>
		/// Get informations about the federation the client is connected to.
		/// </summary>
		/// <param name="ct">Token to cancel the request</param>
		/// <returns>A <c>pplx::task</c> containing the requested information</returns>
		virtual pplx::task<Federation> getFederation(pplx::cancellation_token ct = pplx::cancellation_token::none()) = 0;

		/// <summary>
		/// Set the timeout duration for the server connection.
		/// </summary>
		/// <param name="timeout">Timeout value</param>
		/// <param name="ct">Token to cancel the request</param>
		/// <returns>A <c>pplx::task</c> that completes when the request is done</returns>
		virtual pplx::task<void> setServerTimeout(std::chrono::milliseconds timeout, pplx::cancellation_token ct = pplx::cancellation_token::none()) = 0;
	};
}
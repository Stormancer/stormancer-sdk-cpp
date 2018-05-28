#pragma once

#include "stormancer/headers.h"
#include "stormancer/IPacketDispatcher.h"
#include "stormancer/DefaultScheduler.h"
#include "stormancer/DefaultPacketDispatcher.h"
#include "stormancer/IPlugin.h"
#include "stormancer/IActionDispatcher.h"
#include "stormancer/ITransport.h"
#include "stormancer/Logger/NullLogger.h"

namespace Stormancer
{
	enum class EndpointSelectionMode
	{
		FALLBACK = 0,
		RANDOM = 1
	};

	class Configuration;
	using Configuration_ptr = std::shared_ptr<Configuration>;

	/// Used by a Client for initialization.
	/// For instance to target a custom Stormancer cluster change the ServerEndoint property to the http API endpoint of your custom cluster.
	class Configuration
	{
	public:

		friend class Client;

#pragma region public_methods

		~Configuration();

		/// Create an account with an account and an application name and returns a Configuration smart ptr.
		STORMANCER_DLL_API static Configuration_ptr create(const std::string& endpoint, const std::string& account, const std::string& application);

		/// Add a server endpoint in the internal list
		STORMANCER_DLL_API void addServerEndpoint(const std::string& serverEndpoint);

		/// Get the Api endpoint.
		STORMANCER_DLL_API std::vector<std::string> getApiEndpoint();

		/// Add a plugin to the client.
		/// Plugins enable developpers to plug custom code in the stormancer client's extensibility points. Possible uses include: custom high level protocols, logger or analyzers.
		/// \param plugin The plugin instance to add.
		STORMANCER_DLL_API void addPlugin(IPlugin* plugin);

		/// Get a reference to the plugins list
		STORMANCER_DLL_API const std::vector<IPlugin*> plugins();

#pragma endregion

#pragma region public_members

		/// A string containing the account name of the application.
		const std::string account = "";

		/// A string containing the name of the application.
		const std::string application = "";

		/// Gets or Sets the dispatcher to be used by the client.
		std::function<std::shared_ptr<IPacketDispatcher>(DependencyResolver*)> dispatcher;

		/// Maximum number of remote peers that can connect with this client.
		uint16 maxPeers = 10;

		/// Optional server port
		uint16 serverPort = 0;

		/// Enable or disable the asynchrounous dispatch of received messages. Enabled by default.
		bool asynchronousDispatch = true;

		///Size of the threadpool. Defaults to 5. Less than 3 can provoke deadlocks.
		int threadpoolSize = 10;

		/// The scheduler used by the client to run the transport and other repeated tasks.
		std::shared_ptr<IScheduler> scheduler;

		/// Gets or sets the transport to be used by the client.
		std::function<std::shared_ptr<ITransport>(DependencyResolver*)> transportFactory;

		///Gets or sets the default p2p host port. 0 For automatic attribution.
		unsigned short p2pServerPort = 7777;

		///Gets or sets the default public
		std::string publicIp;

		///Gets or sets 
		bool enableNatPunchthrough = true;

		/// <summary>
		/// Dispatches events
		/// </summary>
		/// <remarks>
		/// By default, events are dispatched on the current network thread. Replace the dispatcher with a MainThreadActionDispatcher to dispatch to your main thread.
		/// your main game loop for instance.
		/// </remarks>
		std::shared_ptr<IActionDispatcher> actionDispatcher = std::make_shared<SameThreadActionDispatcher>();

		/// use the syncClock
		bool synchronisedClock = true;

		/// The interval between successive ping requests, in milliseconds. Default is 5000 ms.
		int32 synchronisedClockInterval = 5000;

		EndpointSelectionMode endpointSelectionMode = EndpointSelectionMode::FALLBACK;

		ILogger_ptr logger = std::make_shared<NullLogger>();

		std::string endpointRootCertificate;

		bool shoudInitializeNetworkLibraries = true;

#pragma endregion

	private:

#pragma region private_methods

		// Constructor.
		Configuration(const std::string& endpoint, const std::string& account, const std::string& application);

		// Copy constructor deleted.
		Configuration(const Configuration& other) = delete;

		// Assignment operator deleted.
		Configuration& operator=(const Configuration& other) = delete;

		// Set a metadata
		Configuration& setMetadata(const std::string& key, const std::string& value);

#pragma endregion

#pragma region private_members

		std::map<std::string, std::string> _metadata;

		std::vector<IPlugin*> _plugins;

		/// A string containing the target server endpoint.
		std::vector<std::string> _serverEndpoints;

		static const std::function<std::shared_ptr<ITransport>(DependencyResolver*)> _defaultTransportFactory;

#pragma endregion
	};
};

#pragma once
#include "headers.h"
#include "IPacketDispatcher.h"
#include "DefaultScheduler.h"
#include "RakNetTransport.h"
#include "DefaultPacketDispatcher.h"
#include "IPlugin.h"
#include "IActionDispatcher.h"

namespace Stormancer
{
	enum class EndpointSelectionMode
	{
		FALLBACK = 0,
		RANDOM = 1
	};

	/// Used by a Client for initialization.
	/// For instance to target a custom Stormancer cluster change the ServerEndoint property to the http API endpoint of your custom cluster.
	class Configuration
	{
		friend class Client;

	private:
		// Constructor.
		Configuration(const std::string endpoint, const std::string account, const std::string application);

		// Copy constructor deleted.
		Configuration(Configuration& other) = delete;

		// Copy deleted.
		Configuration& operator=(Configuration& other) = delete;

	public:
		// Destructor.
		~Configuration();

	public:
		/// Create an account with an account and an application name and returns a Configuration smart ptr.
		STORMANCER_DLL_API static std::shared_ptr<Configuration> create(const std::string endpoint, const std::string account, const std::string application);

		/// Add a server endpoint in the internal list
		STORMANCER_DLL_API void addServerEndpoint(const std::string serverEndpoint);

		// Get the Api endpoint.
		std::vector<std::string> getApiEndpoint();

		/// Add a plugin to the client.
		/// Plugins enable developpers to plug custom code in the stormancer client's extensibility points. Possible uses include: custom high level protocols, logger or analyzers.
		/// \param plugin The plugin instance to add.
		STORMANCER_DLL_API void addPlugin(IPlugin* plugin);

		/// Get a reference to the plugins list
		const std::vector<IPlugin*> plugins();

	private:
		// Set a metadata
		Configuration& metadata(const char*, const char*);

	public:
		/// A string containing the account name of the application.
		const std::string account = "";

		/// A string containing the name of the application.
		const std::string application = "";

		/// Gets or Sets the dispatcher to be used by the client.
		IPacketDispatcher* dispatcher = nullptr;

		/// Maximum number of remote peers that can connect with this client.
		uint16 maxPeers = 0;

		// Optional server port
		uint16 serverPort = 0;

		/// Enable or disable the asynchrounous dispatch of received messages. Enabled by default.
		bool asynchronousDispatch = true;

		/// The scheduler used by the client to run the transport and other repeated tasks.
		std::shared_ptr<IScheduler> scheduler = nullptr;

		/// Gets or sets the transport to be used by the client.
		std::function<std::shared_ptr<ITransport>(DependencyResolver*)> transportFactory;

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

	private:
		const std::function<std::shared_ptr<ITransport>(DependencyResolver*)> defaultTransportFactory = [](DependencyResolver* resolver)
		{

			return std::make_shared<RakNetTransport>(resolver);
		};

		stringMap _metadata;

		std::vector<IPlugin*> _plugins;

		/// A string containing the target server endpoint.
		std::vector<std::string> _serverEndpoints;
	};
};

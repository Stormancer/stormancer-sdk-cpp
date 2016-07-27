#pragma once
#include "headers.h"
#include "IPacketDispatcher.h"
#include "DefaultScheduler.h"
#include "RakNetTransport.h"
#include "DefaultPacketDispatcher.h"
#include "IPlugin.h"

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
		Configuration(const char* account, const char* application);

		// Copy constructor deleted.
		Configuration(Configuration& other) = delete;

		// Copy deleted.
		Configuration& operator=(Configuration& other) = delete;

	public:
		// Destructor.
		~Configuration();

	public:
		/// Create an account with an account and an application name and returns a Configuration smart ptr.
		STORMANCER_DLL_API static Configuration* forAccount(const char* account, const char* application);

		/// Add a server endpoint in the internal list
		STORMANCER_DLL_API void addServerEndpoint(const char* serverEndpoint);

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
		const char* account = "";

		/// A string containing the name of the application.
		const char* application = "";

		/// Gets or Sets the dispatcher to be used by the client.
		IPacketDispatcher* dispatcher = nullptr;

		/// Maximum number of remote peers that can connect with this client.
		uint16 maxPeers = 0;

		/// Enable or disable the asynchrounous dispatch of received messages. Enabled by default.
		bool asynchronousDispatch = true;

		/// The scheduler used by the client to run the transport and other repeated tasks.
		IScheduler* scheduler = nullptr;

		/// Gets or sets the transport to be used by the client.
		std::function<ITransport*(DependencyResolver*)> transportFactory;

		/// <summary>
		/// Dispatches events
		/// </summary>
		/// <remarks>
		/// By default, events are dispatched on the network thread. Replace this function to dispatch events asynchronously to
		/// your main game loop for instance.
		/// </remarks>
		std::function<void(std::function<void(void)>)> eventDispatcher = [](std::function<void(void)> ev) {ev(); };
		/// use the syncClock
		bool synchronisedClock = true;

		/// The interval between successive ping requests, in milliseconds. Default is 5000 ms.
		int32 synchronisedClockInterval = 5000;

		EndpointSelectionMode endpointSelectionMode = EndpointSelectionMode::FALLBACK;

	private:
		const std::function<ITransport*(DependencyResolver*)> defaultTransportFactory = [](DependencyResolver* resolver)
		{

			return new RakNetTransport(resolver);
		};

		stringMap _metadata;

		std::string apiEndpoint = "https://api.stormancer.com/";

		std::vector<IPlugin*> _plugins;

		/// A string containing the target server endpoint.
		std::vector<std::string> _serverEndpoints;
	};
};

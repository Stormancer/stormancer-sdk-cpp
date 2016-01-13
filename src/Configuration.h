#pragma once
#include "headers.h"
#include "IPacketDispatcher.h"
#include "DefaultScheduler.h"
#include "RakNetTransport.h"
#include "DefaultPacketDispatcher.h"
#include "IPlugin.h"

namespace Stormancer
{
	/// Used by a Client for initialization.
    /// For instance to target a custom Stormancer cluster change the ServerEndoint property to the http API endpoint of your custom cluster.
	class Configuration
	{
		friend class Client;

	private:
		// Constructor.
		Configuration(const char* account, const char* application);

	public:
		// Copy constructor deleted.
		Configuration(Configuration& other) = delete;

		// Copy deleted.
		Configuration& operator=(Configuration& other) = delete;

		// Destructor.
		~Configuration();

	public:
		/// Create an account with an account and an application name and returns a Configuration smart ptr.
		STORMANCER_DLL_API static Configuration* forAccount(const char* account, const char* application);

		// Get the Api endpoint.
		std::string getApiEndpoint();

		/// Get a reference to the plugins list
		STORMANCER_DLL_API const std::vector<IPlugin*>& plugins();

		/// Adds a plugin to the client.
		/// Plugins enable developpers to plug custom code in the stormancer client's extensibility points. Possible uses include: custom high level protocols, logger or analyzers.
		/// \param plugin The plugin instance to add.
		STORMANCER_DLL_API void addPlugin(IPlugin* plugin);

		template<typename T>
		void addPlugin()
		{
			addPlugin(new T());
		}

	private:
		// Set a metadata
		Configuration& metadata(const char*, const char*);

	public:

		/// A string containing the account name of the application.
		const char* account = "";

		/// A string containing the name of the application.
		const char* application = "";

		/// A string containing the target server endpoint.
		const char* serverEndpoint = "";

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

		/// use the syncClock
		bool synchronisedClock = true;

		/// The interval between successive ping requests, in milliseconds. Default is 5000 ms.
		int32 synchronisedClockInterval = 5000;

	private:
		const std::function<ITransport*(DependencyResolver*)> defaultTransportFactory = [](DependencyResolver* resolver)
		{
			return new RakNetTransport(resolver);
		};

		stringMap _metadata;

		std::string apiEndpoint = "https://api.stormancer.com/";

		std::vector<IPlugin*> _plugins;
	};
};

#pragma once
#include "headers.h"
#include "IPacketDispatcher.h"
#include "DefaultScheduler.h"
#include "RakNetTransport.h"
#include "DefaultPacketDispatcher.h"
#include "IClientPlugin.h"

namespace Stormancer
{
	/// Used by a Client for initialization.
    /// For instance to target a custom Stormancer cluster change the ServerEndoint property to the http API endpoint of your custom cluster.
	class Configuration
	{
		friend class Client;

	public:
		/// Constructor.
		/// \param account The account id.
		/// \param application The application name.
		STORMANCER_DLL_API Configuration(const char* account, const char* application);

		/// Destructor.
		STORMANCER_DLL_API ~Configuration();

		/// Copy constructor deleted.
		Configuration(Configuration& other) = delete;

		/// Copy deleted.
		Configuration& operator=(Configuration& other) = delete;

		/// Get the Api endpoint.
		std::string getApiEndpoint();

		std::vector<IClientPlugin*>& plugins();

		/// Adds a plugin to the client.
		/// Plugins enable developpers to plug custom code in the stormancer client's extensibility points. Possible uses include: custom high level protocols, logger or analyzers.
		/// \param plugin The plugin instance to add.
		void addPlugin(IClientPlugin* plugin);

	private:

		/// Set a metadata
		Configuration& metadata(std::string key, std::string value);

	public:

		/// A string containing the account name of the application.
		std::string account;

		/// A string containing the name of the application.
		std::string application;

		/// A string containing the target server endpoint.
		std::string serverEndpoint;

		/// Gets or Sets the dispatcher to be used by the client.
		IPacketDispatcher* dispatcher = nullptr;

		/// Maximum number of remote peers that can connect with this client.
		uint16 maxPeers = 10;

		/// Enable or disable the asynchrounous dispatch of received messages. Enabled by default.
		bool asynchronousDispatch = true;

		/// The interval between successive ping requests, in milliseconds. Default is 5000 ms.
		int32 pingInterval = 5000;

		/// The scheduler used by the client to run the transport and other repeated tasks.
		IScheduler* scheduler = nullptr;

		/// Gets or sets the transport to be used by the client.
		std::function<ITransport*(std::map<std::string, void*>)> transportFactory;

		const std::function<ITransport*(std::map<std::string, void*>)> defaultTransportFactory = [](std::map<std::string, void*> parameters)
		{
			return new RakNetTransport(static_cast<ILogger*>(parameters["ILogger"]), static_cast<IScheduler*>(parameters["IScheduler"]));
		};

	private:

		stringMap _metadata;

		std::string apiEndpoint = "https://api.stormancer.com/";

		std::vector<IClientPlugin*> _plugins;
	};
};

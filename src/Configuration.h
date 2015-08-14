#pragma once
#include "headers.h"
#include "ITransport.h"
#include "IPacketDispatcher.h"
#include "Plugins/IClientPlugin.h"

namespace Stormancer
{
	/// Used by a Client for initialization.
    /// For instance to target a custom Stormancer cluster change the ServerEndoint property to the http API endpoint of your custom cluster.
	class Configuration
	{
	public:

		/// Constructor.
		/// \param account The account id.
		/// \param application The application name.
		STORMANCER_DLL_API Configuration(std::string account, std::string application);

		/// Destructor.
		STORMANCER_DLL_API ~Configuration();

		/// Copy constructor deleted.
		Configuration(Configuration& other) = delete;

		/// Copy deleted.
		Configuration& operator=(Configuration& other) = delete;

		/// Get the Api endpoint.
		std::string getApiEndpoint();

		/// Add a plugin in the configuration
		void addPlugin(IClientPlugin* plugin);

	public:

		/// A string containing the account name of the application.
		std::string account;

		/// A string containing the name of the application.
		std::string application;

		/// A string containing the target server endpoint.
		std::string serverEndpoint;

		/// Gets or Sets the dispatcher to be used by the client.
		IPacketDispatcher* dispatcher = nullptr;

		/// Gets or sets the transport to be used by the client.
		ITransport* transport = nullptr;

		/// Maximum number of remote peers that can connect with this client.
		uint16 maxPeers = 20;

		/// Client metadatas.
		stringMap metadata;

		/// Plugins list
		std::vector<IClientPlugin*> plugins;

	private:
		
		/// Api endpoint.
		std::string apiEndpoint = "https://api.stormancer.com/";
	};
};

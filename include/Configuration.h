#pragma once
#include "headers.h"
#include "ITransport.h"
#include "IPacketDispatcher.h"

namespace Stormancer
{
	/*! Configuration object for a Stormancer client.
	Client configurations can be further customized afterwards.
    For instance to target a custom Stormancer cluster change the ServerEndoint property to the http API endpoint of your custom cluster.
	*/
	class Configuration
	{

	public:

		/*! Constructor.
		\param account The account id.
		\param application The application name.
		*/
		STORMANCER_DLL_API Configuration(wstring account, wstring application);

		/// Destructor.
		STORMANCER_DLL_API ~Configuration();

		/// The copy constructor is deleted.
		Configuration(Configuration& other) = delete;

		/// The copy is deleted.
		Configuration& operator=(Configuration& other) = delete;

		wstring getApiEndpoint();

		/*! Adds metadata to connections created by the client.
		\param key A string containing the metadata key.
		\param value A string containing the metadata value.
		\return The current configuration.
		The metadata you provides here will be available on the server to customize its behavior.
		*/
		Configuration& setMetadata(wstring key, wstring value);

		//void addPlugin(IClientPlugin* plugin);

	public:

		/// A string containing the account name of the application.
		wstring account;

		/// A string containing the name of the application.
		wstring application;

		/// A string containing the target server endpoint.
		wstring serverEndpoint;

		/// Gets or Sets the dispatcher to be used by the client.
		IPacketDispatcher* dispatcher = nullptr;

		/// Gets or sets the transport to be used by the client.
		ITransport* transport = nullptr;

		/// Maximum number of remote peers that can connect with this client.
		uint16 maxPeers = 20;

		stringMap metadata;

	private:
		wstring apiEndpoint = L"https://api.stormancer.com/";

		//vector<IClientPlugin*> plugins;

	};
};

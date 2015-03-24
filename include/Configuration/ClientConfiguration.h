#pragma once
#include "headers.h"

namespace Stormancer
{
	class ClientConfiguration
	{
	public:
		ClientConfiguration(string account, string application);
		~ClientConfiguration();

		string getApiEndpoint();
		ClientConfiguration& setMetadata(string key, string value);
		//void addPlugin(shared_ptr<IClientPlugin*> plugin);

	public:
		string account;
		string application;
		string serverEndpoint;
		//shared_ptr<IPacketDispatcher*> dispatcher;
		//ITransport transport;
		//list<ISerializer> serializers;
		uint16 maxPeers;
		//list<shared_ptr<IClientPlugin*>> plugins;
		StringMap metadata;

	private:
		const string apiEndpoint = "http://localhost:23469/";
	};
};

#pragma once
#include "libs.h"
#include "Infrastructure/IPacketDispatcher.h"
#include "ITransport.h"
#include "Core/ISerializer.h"
#include "Plugins/IClientPlugin.h"

namespace Stormancer
{
	class ClientConfiguration
	{
	public:
		ClientConfiguration(string account, string application);
		~ClientConfiguration();

		string getApiEndpoint();
		ClientConfiguration& metadata(string key, string value);
		void addPlugin(shared_ptr<IClientPlugin*> plugin);

	public:
		string account;
		string serverEndpoint;
		string application;
		shared_ptr<IPacketDispatcher*> dispatcher;
		ITransport transport;
		list<ISerializer> serializers;
		uint16 maxPeers;
		list<shared_ptr<IClientPlugin*>> plugins;

	private:
		const string apiEndpoint = "http://localhost:23469/";
		StringMap _metadata;
	};
};

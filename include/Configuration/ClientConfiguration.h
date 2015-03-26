#pragma once
#include "headers.h"

namespace Stormancer
{
	class ClientConfiguration
	{
	public:
		ClientConfiguration(wstring account, wstring application);
		~ClientConfiguration();

		wstring getApiEndpoint();
		ClientConfiguration& setMetadata(wstring key, wstring value);
		//void addPlugin(shared_ptr<IClientPlugin*> plugin);

	public:
		wstring account;
		wstring application;
		wstring serverEndpoint;
		//shared_ptr<IPacketDispatcher*> dispatcher;
		//ITransport transport;
		//list<ISerializer> serializers;
		uint16 maxPeers;
		//list<shared_ptr<IClientPlugin*>> plugins;
		StringMap metadata;

	private:
		wstring apiEndpoint = L"http://localhost:8081/";
	};
};

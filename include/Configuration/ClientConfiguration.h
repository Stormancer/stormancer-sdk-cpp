#pragma once
#include "headers.h"
#include "ITransport.h"
#include "Infrastructure/IPacketDispatcher.h"

namespace Stormancer
{
	class ClientConfiguration
	{
	public:
		ClientConfiguration(wstring account, wstring application);
		~ClientConfiguration();

		wstring getApiEndpoint();
		ClientConfiguration& setMetadata(wstring key, wstring value);
		//void addPlugin(shared_ptr<IClientPlugin> plugin);

	public:
		wstring account;
		wstring application;
		wstring serverEndpoint;
		shared_ptr<IPacketDispatcher> dispatcher;
		shared_ptr<ITransport> transport;
		vector<shared_ptr<ISerializer>> serializers;
		uint16 maxPeers;
		//vector<shared_ptr<IClientPlugin>> plugins;
		stringMap metadata;

	private:
		wstring apiEndpoint = L"http://localhost:8081/";
	};
};

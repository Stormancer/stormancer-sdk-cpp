#pragma once
#include "headers.h"
#include "ITransport.h"
#include "IPacketDispatcher.h"

namespace Stormancer
{
	class Configuration
	{
	public:
		STORMANCER_DLL_API Configuration(wstring account, wstring application);
		STORMANCER_DLL_API ~Configuration();

		Configuration(Configuration& other) = delete;
		Configuration& operator=(Configuration& other) = delete;

		wstring getApiEndpoint();
		Configuration& setMetadata(wstring key, wstring value);
		//void addPlugin(IClientPlugin* plugin);

	public:
		wstring account;
		wstring application;
		wstring serverEndpoint;
		IPacketDispatcher* dispatcher = nullptr;
		ITransport* transport = nullptr;
		uint16 maxPeers = 0;
		//vector<IClientPlugin*> plugins;
		stringMap metadata;

	private:
		wstring apiEndpoint;
	};
};

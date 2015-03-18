#pragma once
#include "stdafx.h"
#include "Infrastructure/IPacketDispatcher.h"
#include "ITransport.h"
#include "Core/ISerializer.h"

namespace Stormancer
{
	class ClientConfiguration
	{
	public:
		~ClientConfiguration();

		static ClientConfiguration forAccount(string account, string application);

	private:
		ClientConfiguration();
		ClientConfiguration(string account = "", string application = "");

	public:
		string account;
		string application;
		string serverEndpoint;
		bool isLocalDev;
		IPacketDispatcher dispatcher;
		ITransport transport;
		list<ISerializer> serializers;
		list<IClientPlugin> plugins;
		uint16 maxPeers;
		ILogger& logger;
	};
};

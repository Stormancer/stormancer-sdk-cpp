#pragma once
#include "headers.h"
#include "Scene.h"
#include "Client.h"

namespace Stormancer
{
	class PluginBuildContext
	{
	public:
		PluginBuildContext();
		virtual ~PluginBuildContext();

		Action<Scene*> sceneCreated;

		Action<Client*> clientCreated;

		Action<Scene*> sceneConnected;

		Action<Scene*> sceneDisconnected;

		Action<Packet<>*> packetReceived;
	};
};

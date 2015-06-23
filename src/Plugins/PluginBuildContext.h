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

		vector<function<void(Scene&)>> sceneCreated;

		vector<function<void(Client&)>> clientCreated;

		vector<function<void(Scene&)>> sceneConnected;

		vector<function<void(Scene&)>> sceneDisconnected;

		vector<function<void(Packet2&)>> packetReceived;
	};
};

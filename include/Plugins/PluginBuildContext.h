#pragma once
#include "stdafx.h"
#include "Scene.h"
#include "Client.h"

namespace Stormancer
{
	class PluginBuildContext
	{
	public:
		PluginBuildContext();
		virtual ~PluginBuildContext();

		function<Scene> sceneCreated;

		function<Client> clientCreated;

		function<Scene> sceneConnected;

		function<Scene> sceneDisconnected;

		function<Packet> packetReceived;
	};
};

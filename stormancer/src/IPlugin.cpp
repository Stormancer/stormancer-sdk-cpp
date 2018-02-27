#include "stormancer/stdafx.h"
#include "stormancer/IPlugin.h"
#include "stormancer/Packet.h"

namespace Stormancer
{
	IPlugin::~IPlugin()
	{
	}

	void IPlugin::clientCreated(Client*)
	{
	}

	void IPlugin::transportStarted(ITransport*)
	{
	}

	void IPlugin::registerSceneDependencies(Scene*)
	{
	}

	void IPlugin::sceneCreated(Scene*)
	{
	}

	void IPlugin::sceneConnecting(Scene*)
	{
	}

	void IPlugin::sceneConnected(Scene*)
	{
	}

	void IPlugin::sceneDisconnecting(Scene*)
	{
	}

	void IPlugin::sceneDisconnected(Scene*)
	{
	}

	void IPlugin::packetReceived(Packet_ptr)
	{
	}
};

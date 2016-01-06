#include "stormancer.h"

namespace Stormancer
{
	IPlugin::~IPlugin()
	{
	}

	void IPlugin::clientCreated(Client* client)
	{
	}

	void IPlugin::transportStarted(ITransport* transport)
	{
	}

	void IPlugin::sceneCreated(Scene* scene)
	{
	}

	void IPlugin::sceneConnecting(Scene* scene)
	{
	}

	void IPlugin::sceneConnected(Scene* scene)
	{
	}

	void IPlugin::sceneDisconnected(Scene* scene)
	{
	}

	void IPlugin::packetReceived(Packet_ptr packet)
	{
	}
};

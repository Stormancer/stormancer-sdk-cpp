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

	void IPlugin::transportEvent(void* data, void* data2, void* data3)
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

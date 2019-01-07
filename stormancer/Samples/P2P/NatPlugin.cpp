#include "NatPlugin.h"

void NatPlugin::clientCreated(Stormancer::Client* client)
{
	_client = client;
}

void NatPlugin::transportStarted(Stormancer::ITransport* transport)
{
	if (_client)
	{
		_natService = new NatService(_client);
		_client->dependencyResolver()->registerDependency(_natService);
		_natService->_transport = transport;
	}
}

void NatPlugin::transportEvent(void* data, void* data2, void* data3)
{
	_natService->transportEvent((Stormancer::byte*)data, (RakNet::Packet*)data2, (RakNet::RakPeerInterface*)data3);
}

void NatPlugin::sceneCreated(Stormancer::Scene* scene)
{
	//
}

void NatPlugin::sceneConnected(Stormancer::Scene* scene)
{
	if (scene)
	{
		auto name = scene->getHostMetadata("stormancer.natPunchthrough");

		if (name && std::strlen(name) > 0 && strcmp(name, "enabled") == 0)
		{
			_natService->_scene = scene;
			_natService->upload();
		}
	}
}

void NatPlugin::sceneDisconnected(Stormancer::Scene* scene)
{
	if (scene)
	{
		auto name = scene->getHostMetadata("stormancer.natPunchthrough");

		if (name && std::strlen(name) > 0 && strcmp(name, "enabled") == 0)
		{
			_natService->_scene = nullptr;
		}
	}
}

void NatPlugin::destroy()
{
	delete this;
}

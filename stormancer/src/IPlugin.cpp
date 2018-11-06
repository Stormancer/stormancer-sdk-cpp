#include "stormancer/stdafx.h"
#include "stormancer/IPlugin.h"
#include "stormancer/IClient.h"
#include "stormancer/scene.h"
#include "stormancer/Packet.h"

namespace Stormancer
{
	IPlugin::~IPlugin()
	{
	}

	void IPlugin::clientCreated(std::shared_ptr<IClient>)
	{
	}

	void IPlugin::transportStarted(std::shared_ptr<ITransport>)
	{
	}

	void IPlugin::registerSceneDependencies(std::shared_ptr<Scene>)
	{
	}

	void IPlugin::sceneCreated(std::shared_ptr<Scene>)
	{
	}

	void IPlugin::sceneConnecting(std::shared_ptr<Scene>)
	{
	}

	void IPlugin::sceneConnected(std::shared_ptr<Scene>)
	{
	}

	void IPlugin::sceneDisconnecting(std::shared_ptr<Scene>)
	{
	}

	void IPlugin::sceneDisconnected(std::shared_ptr<Scene>)
	{
	}

	void IPlugin::packetReceived(std::shared_ptr<Packet<IConnection>>)
	{
	}

	void IPlugin::clientDisconnecting(std::shared_ptr<IClient>)
	{

	}
};

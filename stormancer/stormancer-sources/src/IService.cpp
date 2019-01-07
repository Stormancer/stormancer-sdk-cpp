#include "stormancer/stdafx.h"
#include "stormancer/IService.h"

namespace Stormancer
{
	IService::~IService()
	{
	}

	void IService::setClient(Client*)
	{
	}

	void IService::setTransport(ITransport*)
	{
	}

	void IService::setScene(std::shared_ptr<Scene>)
	{
	}

	void IService::sceneConnected(std::shared_ptr<Scene>)
	{
	}

	void IService::packetReceived(Packet_ptr)
	{
	}
}

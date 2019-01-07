#pragma once
#include <stormancer.h>
#include "NatService.h"

class NatPlugin : public Stormancer::IPlugin
{
public:
	void clientCreated(Stormancer::Client* client);
	void transportStarted(Stormancer::ITransport* transport);
	void transportEvent(void* data, void* data2, void* data3);
	void sceneCreated(Stormancer::Scene* scene);
	void sceneConnected(Stormancer::Scene* scene);
	void sceneDisconnected(Stormancer::Scene* scene);
	void destroy();

private:
	Stormancer::Client* _client = nullptr;
	NatService* _natService = nullptr;
};

#pragma once

#include "stormancer/headers.h"
#include "stormancer/Packet.h"

namespace Stormancer
{
	class Client;
	class Scene;
	class ITransport;

	class IPlugin
	{
	public:

#pragma region public_methods

		STORMANCER_DLL_API virtual ~IPlugin();
		STORMANCER_DLL_API virtual void clientCreated(Client* client);
		STORMANCER_DLL_API virtual void transportStarted(ITransport* transport);
		STORMANCER_DLL_API virtual void registerSceneDependencies(Scene* scene);
		STORMANCER_DLL_API virtual void sceneCreated(Scene* scene);
		STORMANCER_DLL_API virtual void sceneConnecting(Scene* scene);
		STORMANCER_DLL_API virtual void sceneConnected(Scene* scene);
		STORMANCER_DLL_API virtual void sceneDisconnecting(Scene* scene);
		STORMANCER_DLL_API virtual void sceneDisconnected(Scene* scene);
		STORMANCER_DLL_API virtual void packetReceived(Packet_ptr packet);

#pragma endregion
	};
};

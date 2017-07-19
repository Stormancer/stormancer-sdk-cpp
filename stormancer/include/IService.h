#pragma once

#include "headers.h"
#include "Packet.h"

namespace Stormancer
{
	class Client;
	class Scene;
	class ITransport;

	class IService
	{
	public:

#pragma region public_methods

		STORMANCER_DLL_API virtual ~IService();
		STORMANCER_DLL_API virtual void setClient(Client* client);
		STORMANCER_DLL_API virtual void setTransport(ITransport* transport);
		STORMANCER_DLL_API virtual void setScene(Scene* scene);
		STORMANCER_DLL_API virtual void packetReceived(Packet_ptr packet);

#pragma endregion
	};
};

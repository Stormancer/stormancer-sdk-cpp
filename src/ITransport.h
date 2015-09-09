#pragma once
#include "headers.h"
#include "IConnectionManager.h"
#include "Packet.h"
#include "Helpers.h"
#include "Action.h"

namespace Stormancer
{
	/// Interface for a network transport.
	class ITransport
	{
	public:
		ITransport();
		virtual ~ITransport();

		virtual void start(std::string name, IConnectionManager* handler, pplx::cancellation_token token, uint16 port = 0, uint16 maxConnections = 0) = 0;
		virtual pplx::task<IConnection*> connect(std::string endpoint) = 0;
		bool isRunning();
		std::string name();
		uint64 id();

	public:
		Action<std::shared_ptr<Packet<>>> packetReceived;
		Action<IConnection*> connectionOpened;
		Action<IConnection*> connectionClosed;

	protected:
		bool _isRunning = false;
		std::string _name;
		uint64 _id = 0;
	};
};

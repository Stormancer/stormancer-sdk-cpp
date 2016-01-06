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
		virtual void start(std::string name, IConnectionManager* handler, pplx::cancellation_token token, uint16 port = 0, uint16 maxConnections = 0) = 0;
		virtual pplx::task<IConnection*> connect(std::string endpoint) = 0;
		virtual bool isRunning() const = 0;
		virtual std::string name() const = 0;
		virtual uint64 id() const = 0;
		virtual DependencyResolver* dependencyResolver() const = 0;
		virtual void onPacketReceived(std::function<void(Packet_ptr)> callback) = 0;
		virtual const char* host() const = 0;
		virtual uint16 port() const = 0;
	};
};

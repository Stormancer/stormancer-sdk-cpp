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

#pragma region public_methods

		virtual ~ITransport() {}
		virtual void start(std::string name, std::shared_ptr<IConnectionManager> handler, pplx::cancellation_token token = pplx::cancellation_token::none(), uint16 port = 0, uint16 maxConnections = 0) = 0;
		virtual pplx::task<std::weak_ptr<IConnection>> connect(std::string endpoint) = 0;
		virtual bool isRunning() const = 0;
		virtual std::string name() const = 0;
		virtual uint64 id() const = 0;
		virtual DependencyResolver* dependencyResolver() const = 0;
		virtual void onPacketReceived(std::function<void(Packet_ptr)> callback) = 0;
		virtual std::string host() const = 0;
		virtual uint16 port() const = 0;
		virtual std::vector<std::string> externalAddresses() const = 0;
		virtual std::vector<std::string> getAvailableEndpoints() const = 0;
		virtual pplx::task<int> sendPing(const std::string& address) = 0;
		virtual void openNat(const std::string& address) = 0;

#pragma endregion
	};
};

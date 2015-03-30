#pragma once
#include "headers.h"
#include "Core/IConnection.h"

namespace Stormancer
{
	class IConnectionManager
	{
	public:
		IConnectionManager();
		virtual ~IConnectionManager();

		virtual uint64 generateNewConnectionId() = 0;
		virtual void newConnection(shared_ptr<IConnection> connection) = 0;
		virtual void closeConnection(shared_ptr<IConnection> connection, wstring reason) = 0;
		virtual shared_ptr<IConnection> getConnection(uint64 id) = 0;

	public:
		int connectionCount = 0;
	};
};

#include "stormancer.h"

namespace Stormancer
{
	RakNetTransport::RakNetTransport(ILogger* logger)
		: _logger(logger)
	{
		ITransport::_name = L"raknet";
	}

	RakNetTransport::~RakNetTransport()
	{
	}

	pplx::task<void> RakNetTransport::start(wstring type, IConnectionManager* handler, pplx::cancellation_token token, uint16 maxConnections, uint16 serverPort)
	{
		if (handler == nullptr && serverPort > 0)
		{
			throw string("Handler is null.");
		}
		_type = type;

		auto tce = pplx::task_completion_event<void>();
		pplx::task<void>([this, &token, &tce, maxConnections]() {
			this->run(token, tce, maxConnections);
		});

		_handler = handler;
		return pplx::create_task(tce);
	}

	void RakNetTransport::run(pplx::cancellation_token token, pplx::task_completion_event<void> startupTce, uint16 maxConnections, uint16 serverPort)
	{
		_isRunning = true;
		_logger->log(LogLevel::Info, L"", Helpers::StringFormat(L"Starting raknet transport {0}", _type), L"");
		auto server = RakNet::RakPeerInterface::GetInstance();
		auto socketDescriptor = (serverPort != 0 ? new RakNet::SocketDescriptor(serverPort, nullptr) : new RakNet::SocketDescriptor());
		auto startupResult = server->Startup(maxConnections, socketDescriptor, 1);
		if (startupResult != RakNet::StartupResult::RAKNET_STARTED)
		{
			throw string(Helpers::StringFormat(L"Couldn't start raknet peer : {0}", startupResult));
		}
		server->SetMaximumIncomingConnections(maxConnections);

		_peer = server;
		startupTce.set();
		_logger->log(LogLevel::Info, L"", Helpers::StringFormat(L"Raknet transport started {0}", _type), L"");
		while (!token.is_canceled())
		{
			RakNet::Packet* packet;
			while ((packet = server->Receive()) != nullptr)
			{
				if (packet->length == 0)
				{
					continue;
				}

				wstring packetSystemAddressStr = Helpers::to_wstring(packet->systemAddress.ToString());
				switch (packet->data[0])
				{
				case (int)DefaultMessageIDTypes::ID_CONNECTION_REQUEST_ACCEPTED:
				{
					if (Helpers::mapContains(_pendingConnections, packetSystemAddressStr))
					{
						auto tce = _pendingConnections[packetSystemAddressStr];
						auto c = createNewConnection(packet->guid, server);
						tce.set(c);
					}
					_logger->log(LogLevel::Debug, L"", Helpers::StringFormat(L"Connection request to {0} accepted.", packetSystemAddressStr), L"");
					onConnection(packet, server);
					break;
				}
				case (int)DefaultMessageIDTypes::ID_NEW_INCOMING_CONNECTION:
				{
					_logger->log(LogLevel::Trace, L"", Helpers::StringFormat(L"Icoming connection from {0}.", packetSystemAddressStr), L"");
					onConnection(packet, server);
					break;
				}
				case (int)DefaultMessageIDTypes::ID_DISCONNECTION_NOTIFICATION:
				{
					_logger->log(LogLevel::Trace, L"", Helpers::StringFormat(L"{0} disconnected.", packetSystemAddressStr), L"");
					onDisconnection(packet, server, L"CLIENT_DISCONNECTED");
					break;
				}
				case (int)DefaultMessageIDTypes::ID_CONNECTION_LOST:
				{
					_logger->log(LogLevel::Trace, L"", Helpers::StringFormat(L"{0} lost the connection.", packetSystemAddressStr), L"");
					onDisconnection(packet, server, L"CONNECTION_LOST");
					break;
				}
				case (int)MessageIDTypes::ID_CONNECTION_RESULT:
				{
					onConnectionIdReceived(*(static_cast<int64*>(static_cast<void*>(packet->data + 1))));
					break;
				}
				case (int)DefaultMessageIDTypes::ID_CONNECTION_ATTEMPT_FAILED:
					if (Helpers::mapContains(_pendingConnections, packetSystemAddressStr))
					{
						auto tce = _pendingConnections[packetSystemAddressStr];
						exception_ptr eptr = make_exception_ptr("Connection attempt failed.");
						tce.set_exception(eptr);
					}
					break;
				default:
				{
					onMessageReceived(packet);
					break;
				}
				}
			}
			Sleep(0);
		}
		server->Shutdown(1000);
		_isRunning = false;
		_logger->log(LogLevel::Info, L"", L"Stopped raknet server.", L"");
	}

	void RakNetTransport::onConnectionIdReceived(uint64 p)
	{
		_id = p;
	}

	pplx::task<IConnection*> RakNetTransport::connect(wstring endpoint)
	{
		if (_peer == nullptr || !_peer->IsActive())
		{
			throw string("Transport not started. Call start before connect.");
		}
		auto infos = Helpers::stringSplit(endpoint, L":");
		auto host = Helpers::to_string(infos[0]).c_str();
		uint16 port = infos[1][0] * 256 + infos[1][1];
		_peer->Connect(host, port, nullptr, 0);

		auto addressStr = Helpers::to_wstring(RakNet::SystemAddress(host, port).ToString());

		_pendingConnections[addressStr] = pplx::task_completion_event<IConnection*>();
		auto& tce = _pendingConnections[addressStr];

		return pplx::task<IConnection*>(tce);
	}

	void RakNetTransport::onConnection(RakNet::Packet* packet, RakNet::RakPeerInterface* server)
	{
		wstring sysAddr = Helpers::to_wstring(packet->systemAddress.ToString());
		_logger->log(LogLevel::Trace, L"", Helpers::StringFormat(L"{0} connected.", sysAddr), L"");

		auto c = createNewConnection(packet->guid, server);
		server->DeallocatePacket(packet);
		_handler->newConnection(c);
		if (connectionOpened != nullptr)
		{
			connectionOpened(c);
		}

		c->sendSystem((byte)MessageIDTypes::ID_CONNECTION_RESULT, [c](bytestream* stream) {
			*stream << c->id();
		});
	}

	void RakNetTransport::onDisconnection(RakNet::Packet* packet, RakNet::RakPeerInterface* server, wstring reason)
	{
		wstring sysAddr = Helpers::to_wstring(packet->systemAddress.ToString());
		_logger->log(LogLevel::Trace, L"", Helpers::StringFormat(L"{0} disconnected.", sysAddr), L"");
		auto c = removeConnection(packet->guid);
		server->DeallocatePacket(packet);

		_handler->closeConnection(c, reason);

		if (connectionClosed != nullptr)
		{
			connectionClosed(c);
		}
		c->connectionClosed(reason);
	}

	void RakNetTransport::onMessageReceived(RakNet::Packet* packet)
	{
		auto connection = getConnection(packet->guid);
		stringstream* stream = Helpers::convertRakNetPacketToStringStream(packet, _peer);
		auto p = new Packet<>(connection, stream);
		p->clean = [this, &packet]() {
			if (this->_peer != nullptr && packet != nullptr)
			{
				this->_peer->DeallocatePacket(packet);
			}
		};

		_logger->log(LogLevel::Trace, L"", Helpers::StringFormat(L"Message with id {0} arrived.", (byte)packet->data[0]), L"");

		packetReceived(p);
	}

	RakNetConnection* RakNetTransport::getConnection(RakNet::RakNetGUID guid)
	{
		return _connections[guid.g];
	}

	RakNetConnection* RakNetTransport::createNewConnection(RakNet::RakNetGUID raknetGuid, RakNet::RakPeerInterface* peer)
	{
		int64 cid = _handler->generateNewConnectionId();
		function<void(RakNetConnection*)> lambdaOnRequestClose = [this](RakNetConnection* c) {
			this->onRequestClose(c);
		};
		auto c = new RakNetConnection(raknetGuid, cid, peer, lambdaOnRequestClose);
		_connections[raknetGuid.g] = c;
		return c;
	}

	RakNetConnection* RakNetTransport::removeConnection(RakNet::RakNetGUID guid)
	{
		RakNetConnection* connection = _connections[guid.g];
		_connections.erase(guid.g);
		return connection;
	}

	void RakNetTransport::onRequestClose(RakNetConnection* c)
	{
		_peer->CloseConnection(c->guid(), true);
	}
};

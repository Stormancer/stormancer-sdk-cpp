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
		if (_handler)
		{
			delete _handler;
		}
	}

	pplx::task<void> RakNetTransport::start(wstring type, IConnectionManager* handler, pplx::cancellation_token token, uint16 maxConnections, uint16 serverPort)
	{
		if (handler == nullptr && serverPort > 0)
		{
			throw exception("Handler is null.");
		}
		_type = type;

		auto tce = pplx::task_completion_event<void>();
		pplx::task<void>([this, token, tce, maxConnections]() {
			this->run(token, tce, maxConnections);
		});

		_handler = handler;
		return pplx::create_task(tce);
	}

	void RakNetTransport::run(pplx::cancellation_token token, pplx::task_completion_event<void> startupTce, uint16 maxConnections, uint16 serverPort)
	{
		_isRunning = true;
		_logger->log(LogLevel::Info, L"", StringFormat(L"Starting raknet transport {0}", _type), L"");
		auto server = RakNet::RakPeerInterface::GetInstance();
		auto socketDescriptor = (serverPort != 0 ? new RakNet::SocketDescriptor(serverPort, nullptr) : new RakNet::SocketDescriptor());
		auto startupResult = server->Startup(maxConnections, socketDescriptor, 1);
		if (startupResult != RakNet::StartupResult::RAKNET_STARTED)
		{
			throw exception(string(StringFormat(L"Couldn't start raknet peer : {0}", startupResult)).c_str());
		}
		server->SetMaximumIncomingConnections(maxConnections);

		_peer = server;
		startupTce.set();
		_logger->log(LogLevel::Info, L"", StringFormat(L"Raknet transport started {0}", _type), L"");
		RakNet::Packet* rakNetPacket;
		wstring packetSystemAddressStr;
		while (!token.is_canceled())
		{
			while ((rakNetPacket = server->Receive()) != nullptr)
			{
				if (rakNetPacket->length == 0)
				{
					continue;
				}

				packetSystemAddressStr = Helpers::to_wstring(rakNetPacket->systemAddress.ToString());
				switch (rakNetPacket->data[0])
				{
				case (byte)DefaultMessageIDTypes::ID_CONNECTION_REQUEST_ACCEPTED:
				{
					if (Helpers::mapContains(_pendingConnections, packetSystemAddressStr))
					{
						auto tce = _pendingConnections[packetSystemAddressStr];
						auto c = createNewConnection(rakNetPacket->guid, server);
						tce.set(c);
						_logger->log(LogLevel::Debug, L"", StringFormat(L"Connection request to {0} accepted.", packetSystemAddressStr), L"");
					}
					break;
				}
				case (byte)DefaultMessageIDTypes::ID_NEW_INCOMING_CONNECTION:
				{
					_logger->log(LogLevel::Trace, L"", StringFormat(L"Icoming connection from {0}.", packetSystemAddressStr), L"");
					onConnection(rakNetPacket, server);
					break;
				}
				case (byte)DefaultMessageIDTypes::ID_DISCONNECTION_NOTIFICATION:
				{
					_logger->log(LogLevel::Trace, L"", StringFormat(L"{0} disconnected.", packetSystemAddressStr), L"");
					onDisconnection(rakNetPacket, server, L"CLIENT_DISCONNECTED");
					break;
				}
				case (byte)DefaultMessageIDTypes::ID_CONNECTION_LOST:
				{
					_logger->log(LogLevel::Trace, L"", StringFormat(L"{0} lost the connection.", packetSystemAddressStr), L"");
					onDisconnection(rakNetPacket, server, L"CONNECTION_LOST");
					break;
				}
				case (byte)MessageIDTypes::ID_CONNECTION_RESULT:
				{
					onConnectionIdReceived(*((int64*)(rakNetPacket->data + 1)));
					break;
				}
				case (byte)DefaultMessageIDTypes::ID_CONNECTION_ATTEMPT_FAILED:
				{
					if (Helpers::mapContains(_pendingConnections, packetSystemAddressStr))
					{
						auto tce = _pendingConnections[packetSystemAddressStr];
						exception_ptr eptr = make_exception_ptr("Connection attempt failed.");
						tce.set_exception(eptr);
					}
					break;
				}
				default:
				{
					onMessageReceived(rakNetPacket);
					break;
				}
				}
			}
			Sleep(0);
		}
		server->Shutdown(0);
		_isRunning = false;
		delete socketDescriptor;
		ILogger::instance()->log(LogLevel::Info, L"", L"Stopped raknet server.", L"");
	}

	void RakNetTransport::onConnectionIdReceived(uint64 p)
	{
		_id = p;
	}

	pplx::task<IConnection*> RakNetTransport::connect(wstring endpoint)
	{
		if (_peer == nullptr || !_peer->IsActive())
		{
			throw exception("Transport not started. Call start before connect.");
		}
		auto infos = Helpers::stringSplit(endpoint, L":");
		auto host = Helpers::to_string(infos[0]);
		auto hostCStr = host.c_str();
		uint16 port = static_cast<uint16>(stoi(infos[1]));
		_peer->Connect(hostCStr, port, nullptr, 0);

		auto addressStr = Helpers::to_wstring(RakNet::SystemAddress(hostCStr, port).ToString());

		_pendingConnections[addressStr] = pplx::task_completion_event<IConnection*>();
		auto& tce = _pendingConnections[addressStr];

		return pplx::task<IConnection*>(tce);
	}

	void RakNetTransport::onConnection(RakNet::Packet* rakNetPacket, RakNet::RakPeerInterface* server)
	{
		wstring sysAddr = Helpers::to_wstring(rakNetPacket->systemAddress.ToString());
		_logger->log(LogLevel::Trace, L"", StringFormat(L"{0} connected.", sysAddr), L"");

		auto c = createNewConnection(rakNetPacket->guid, server);
		server->DeallocatePacket(rakNetPacket);
		_handler->newConnection(c);
		connectionOpened(dynamic_cast<IConnection*>(c));

		c->sendSystem((byte)MessageIDTypes::ID_CONNECTION_RESULT, [c](bytestream* stream) {
			*stream << c->id();
		});
	}

	void RakNetTransport::onDisconnection(RakNet::Packet* packet, RakNet::RakPeerInterface* server, wstring reason)
	{
		wstring sysAddr = Helpers::to_wstring(packet->systemAddress.ToString());
		_logger->log(LogLevel::Trace, L"", StringFormat(L"{0} disconnected.", sysAddr), L"");
		auto c = removeConnection(packet->guid);
		server->DeallocatePacket(packet);

		_handler->closeConnection(c, reason);

		connectionClosed(dynamic_cast<IConnection*>(c));
		c->connectionClosed(reason);
		delete c;
	}

	void RakNetTransport::onMessageReceived(RakNet::Packet* rakNetPacket)
	{
		_logger->log(LogLevel::Trace, L"", StringFormat(L"Message with id {0} arrived.", (byte)rakNetPacket->data[0]), L"");

		auto connection = getConnection(rakNetPacket->guid);
		auto stream = new bytestream;
		stream->rdbuf()->pubsetbuf((char*)rakNetPacket->data, rakNetPacket->length);
		shared_ptr<Packet<>> packet( new Packet<>(connection, stream) );
		auto peer = this->_peer;
		packet->cleanup += new function<void(void)>([stream, peer, rakNetPacket]() {
			if (stream)
			{
				stream->rdbuf()->pubsetbuf(nullptr, 0);
				delete stream;
			}
			if (peer && rakNetPacket)
			{
				peer->DeallocatePacket(rakNetPacket);
			}
		});

		packetReceived(packet);
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

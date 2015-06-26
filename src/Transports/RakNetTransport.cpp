#include "stormancer.h"

namespace Stormancer
{
	RakNetTransport::RakNetTransport(ILogger* logger)
		: _logger(logger)
	{
		ITransport::_name = "raknet";
	}

	RakNetTransport::~RakNetTransport()
	{
		if (_handler)
		{
			delete _handler;
		}
	}

	pplx::task<void> RakNetTransport::start(std::string type, IConnectionManager* handler, pplx::cancellation_token token, uint16 maxConnections, uint16 serverPort)
	{
		if (handler == nullptr && serverPort > 0)
		{
			throw std::exception("Handler is null.");
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
		_logger->log(LogLevel::Info, "", Helpers::stringFormat("Starting raknet transport ", _type), "");
		auto server = RakNet::RakPeerInterface::GetInstance();
		auto socketDescriptor = (serverPort != 0 ? new RakNet::SocketDescriptor(serverPort, nullptr) : new RakNet::SocketDescriptor());
		auto startupResult = server->Startup(maxConnections, socketDescriptor, 1);
		if (startupResult != RakNet::StartupResult::RAKNET_STARTED)
		{
			throw std::exception(Helpers::stringFormat("Couldn't start raknet peer : ", startupResult).c_str());
		}
		server->SetMaximumIncomingConnections(maxConnections);

		_peer = server;
		startupTce.set();
		_logger->log(LogLevel::Info, "", Helpers::stringFormat("Raknet transport started ", _type), "");
		RakNet::Packet* rakNetPacket;
		std::string packetSystemAddressStr;
		while (!token.is_canceled())
		{
			while ((rakNetPacket = server->Receive()) != nullptr)
			{
				if (rakNetPacket->length == 0)
				{
					continue;
				}

				packetSystemAddressStr = rakNetPacket->systemAddress.ToString();
				switch (rakNetPacket->data[0])
				{
				case (byte)DefaultMessageIDTypes::ID_CONNECTION_REQUEST_ACCEPTED:
				{
					if (Helpers::mapContains(_pendingConnections, packetSystemAddressStr))
					{
						auto tce = _pendingConnections[packetSystemAddressStr];
						auto c = createNewConnection(rakNetPacket->guid, server);
						tce.set(c);
						_logger->log(LogLevel::Debug, "", Helpers::stringFormat("Connection request to ", packetSystemAddressStr, " accepted."), "");
					}
					break;
				}
				case (byte)DefaultMessageIDTypes::ID_NEW_INCOMING_CONNECTION:
				{
					_logger->log(LogLevel::Trace, "", Helpers::stringFormat("Incoming connection from ", packetSystemAddressStr, "."), "");
					onConnection(rakNetPacket, server);
					break;
				}
				case (byte)DefaultMessageIDTypes::ID_DISCONNECTION_NOTIFICATION:
				{
					_logger->log(LogLevel::Trace, "", Helpers::stringFormat(packetSystemAddressStr, " disconnected."), "");
					onDisconnection(rakNetPacket, server, "CLIENT_DISCONNECTED");
					break;
				}
				case (byte)DefaultMessageIDTypes::ID_CONNECTION_LOST:
				{
					_logger->log(LogLevel::Trace, "", Helpers::stringFormat(packetSystemAddressStr, " lost the connection."), "");
					onDisconnection(rakNetPacket, server, "CONNECTION_LOST");
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
						std::exception_ptr eptr = std::make_exception_ptr("Connection attempt failed.");
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
		ILogger::instance()->log(LogLevel::Info, "", "Stopped raknet server.", "");
	}

	void RakNetTransport::onConnectionIdReceived(uint64 p)
	{
		_id = p;
	}

	pplx::task<IConnection*> RakNetTransport::connect(std::string endpoint)
	{
		if (_peer == nullptr || !_peer->IsActive())
		{
			throw std::exception("Transport not started. Call start before connect.");
		}
		auto infos = Helpers::stringSplit(std::wstring(endpoint.begin(), endpoint.end()), L":");
		std::string host(infos[0].begin(), infos[0].end());
		auto hostCStr = host.c_str();
		uint16 port = static_cast<uint16>(stoi(infos[1]));
		_peer->Connect(hostCStr, port, nullptr, 0);

		std::string addressStr(RakNet::SystemAddress(hostCStr, port).ToString());
		_pendingConnections[addressStr] = pplx::task_completion_event<IConnection*>();
		auto& tce = _pendingConnections[addressStr];

		return pplx::task<IConnection*>(tce);
	}

	void RakNetTransport::onConnection(RakNet::Packet* rakNetPacket, RakNet::RakPeerInterface* server)
	{
		std::string sysAddr(rakNetPacket->systemAddress.ToString());
		_logger->log(LogLevel::Trace, "", Helpers::stringFormat(sysAddr, " connected."), "");

		auto c = createNewConnection(rakNetPacket->guid, server);
		server->DeallocatePacket(rakNetPacket);
		_handler->newConnection(c);
		connectionOpened(dynamic_cast<IConnection*>(c));

		c->sendSystem((byte)MessageIDTypes::ID_CONNECTION_RESULT, [c](bytestream* stream) {
			*stream << c->id();
		});
	}

	void RakNetTransport::onDisconnection(RakNet::Packet* packet, RakNet::RakPeerInterface* server, std::string reason)
	{
		std::string sysAddr(packet->systemAddress.ToString());
		_logger->log(LogLevel::Trace, "", Helpers::stringFormat(sysAddr, " disconnected."), "");
		auto c = removeConnection(packet->guid);
		server->DeallocatePacket(packet);

		_handler->closeConnection(c, reason);

		connectionClosed(dynamic_cast<IConnection*>(c));
		c->connectionClosed(reason);
		delete c;
	}

	void RakNetTransport::onMessageReceived(RakNet::Packet* rakNetPacket)
	{
		_logger->log(LogLevel::Trace, "", Helpers::stringFormat("Message with id ", (byte)rakNetPacket->data[0], " arrived."), "");

		auto connection = getConnection(rakNetPacket->guid);
		auto stream = new bytestream;
		stream->rdbuf()->pubsetbuf((char*)rakNetPacket->data, rakNetPacket->length);
		std::shared_ptr<Packet<>> packet(new Packet<>(connection, stream));
		auto peer = this->_peer;
		packet->cleanup += new std::function<void(void)>([stream, peer, rakNetPacket]() {
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
		auto lambdaOnRequestClose = new std::function<void(RakNetConnection*)>([this](RakNetConnection* c) {
			this->onRequestClose(c);
		});
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

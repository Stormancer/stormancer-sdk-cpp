#include "stormancer.h"
#if defined(_WIN32)
//
#else
#include <unistd.h>
#endif

namespace Stormancer
{
	RakNetTransport::RakNetTransport(ILogger* logger, IScheduler* scheduler)
		: _logger(logger),
		_scheduler(scheduler)
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

	void RakNetTransport::start(std::string type, IConnectionManager* handler, pplx::cancellation_token token, uint16 maxConnections, uint16 serverPort)
	{
		if (handler == nullptr && serverPort > 0)
		{
			throw std::invalid_argument("Handler is null.");
		}
		_type = type;
		_handler = handler;
		initialize(serverPort, maxConnections);
		_scheduledTransportLoop = _scheduler->schedulePeriodic(15, Action<>(std::function<void()>([this]() {
			run();
		})));
	}

	void RakNetTransport::initialize(uint16 maxConnections, uint16 serverPort)
	{
		try
		{
			_isRunning = true;
			_logger->log(LogLevel::Info, "", std::string("Starting raknet transport ") + _type, "");
			_peer = RakNet::RakPeerInterface::GetInstance();
			_socketDescriptor = (serverPort != 0 ? new RakNet::SocketDescriptor(serverPort, nullptr) : new RakNet::SocketDescriptor());
			auto startupResult = _peer->Startup(maxConnections, _socketDescriptor, 1);
			if (startupResult != RakNet::StartupResult::RAKNET_STARTED)
			{
				throw std::runtime_error(std::string("RakNet peer startup failed (RakNet::StartupResult == ") + std::to_string(startupResult) + ')');
			}
			_peer->SetMaximumIncomingConnections(maxConnections);
			_logger->log(LogLevel::Info, "", std::string("Raknet transport started ") + _type, "");
		}
		catch (const std::exception& e)
		{
			throw std::runtime_error(std::string(e.what()) + "\nFailed to initialize the RakNet peer.");
		}
	}

	void RakNetTransport::run()
	{
		try
		{
			RakNet::Packet* rakNetPacket;
			while ((rakNetPacket = _peer->Receive()) != nullptr)
			{
				if (rakNetPacket->length == 0)
				{
					continue;
				}

				try
				{
					switch (rakNetPacket->data[0])
					{
					case (byte)DefaultMessageIDTypes::ID_CONNECTION_REQUEST_ACCEPTED:
					{
						std::string packetSystemAddressStr = rakNetPacket->systemAddress.ToString();
						if (mapContains(_pendingConnections, packetSystemAddressStr))
						{
							auto tce = _pendingConnections[packetSystemAddressStr];
							auto c = createNewConnection(rakNetPacket->guid, _peer);
							tce.set(c);
						}
						_logger->log(LogLevel::Debug, "", std::string("Connection request to ") + packetSystemAddressStr + " accepted.", "");
						onConnection(rakNetPacket, _peer);
						break;
					}
					case (byte)DefaultMessageIDTypes::ID_NEW_INCOMING_CONNECTION:
					{
						_logger->log(LogLevel::Trace, "", std::string("Incoming connection from ") + rakNetPacket->systemAddress.ToString() + ".", "");
						onConnection(rakNetPacket, _peer);
						break;
					}
					case (byte)DefaultMessageIDTypes::ID_DISCONNECTION_NOTIFICATION:
					{
						_logger->log(LogLevel::Trace, "", std::string(rakNetPacket->systemAddress.ToString()) + " disconnected.", "");
						onDisconnection(rakNetPacket, _peer, "CLIENT_DISCONNECTED");
						break;
					}
					case (byte)DefaultMessageIDTypes::ID_CONNECTION_LOST:
					{
						_logger->log(LogLevel::Trace, "", std::string(rakNetPacket->systemAddress.ToString()) + " lost the connection.", "");
						onDisconnection(rakNetPacket, _peer, "CONNECTION_LOST");
						break;
					}
					case (byte)DefaultMessageIDTypes::ID_CONNECTION_ATTEMPT_FAILED:
					{
						std::string packetSystemAddressStr = rakNetPacket->systemAddress.ToString();
						if (mapContains(_pendingConnections, packetSystemAddressStr))
						{
							auto tce = _pendingConnections[packetSystemAddressStr];
							tce.set_exception<std::exception>(std::runtime_error("Connection attempt failed."));
						}
						break;
					}
					case (byte)MessageIDTypes::ID_CONNECTION_RESULT:
					{
						int64 sid = 0;
						for (int i = 0; i < 8; i++)
						{
							sid <<= 8;
							sid += rakNetPacket->data[8 - i];
						}
						_logger->log(LogLevel::Debug, "RakNetTransport::run", "Connection ID received", std::to_string(sid));
						onConnectionIdReceived(sid);
						break;
					}
					case (byte)DefaultMessageIDTypes::ID_ALREADY_CONNECTED:
						throw std::logic_error("Peer already connected");
						break;
					default:
					{
						onMessageReceived(rakNetPacket);
						break;
					}
					}
				}
				catch (std::exception e)
				{
					_logger->log(LogLevel::Error, "transports.raknet", "An error occured while handling a message", e.what());
				}
			}
		}
		catch (std::exception e)
		{
			_logger->log(LogLevel::Error, "transports.raknet", "An error occured while running the transport : {0}", e.what());
		}
	}

	void RakNetTransport::stop()
	{
		if (_isRunning)
		{
			_scheduledTransportLoop->unsubscribe();
			if (_peer)
			{
				if (_peer->IsActive())
				{
					_peer->Shutdown(0);
				}
				RakNet::RakPeerInterface::DestroyInstance(_peer);
				_peer = nullptr;
			}
			if (_socketDescriptor)
			{
				delete _socketDescriptor;
				_socketDescriptor = nullptr;
			}
			_isRunning = false;
			ILogger::instance()->log(LogLevel::Info, "", "Stopped raknet server.", "");
		}
	}

	void RakNetTransport::onConnectionIdReceived(uint64 p)
	{
		_id = p;
	}

	pplx::task<IConnection*> RakNetTransport::connect(std::string endpoint)
	{
		//"^(([0-9A-F]{1,4}:){7}[0-9A-F]{1,4}|(\\d{1,3}\\.){3}\\d{1,3}):(\\d{1,5})$"

		std::smatch m;
		std::regex ipRegex("^((\\d{1,3}\\.){3}\\d{1,3}):(\\d{1,5})$", std::regex_constants::ECMAScript | std::regex_constants::icase);
		bool res = std::regex_search(endpoint, m, ipRegex);
		if (!res || m.length() < 5)
		{
			throw std::invalid_argument("Bad scene endpoint (" + endpoint + ')');
		}

		auto host = m.str(1);
		auto hostCstr = host.c_str();
		uint16 port = (uint16)std::atoi(m.str(3).c_str());
		if (port == 0)
		{
			throw std::runtime_error("Scene endpoint port should not be 0 (" + endpoint + ')');
		}

		if (_peer == nullptr || !_peer->IsActive())
		{
			throw std::runtime_error("Transport not started. Check you started it.");
		}

		auto result = _peer->Connect(hostCstr, port, nullptr, 0);
		if (result != RakNet::ConnectionAttemptResult::CONNECTION_ATTEMPT_STARTED)
		{
			throw std::runtime_error(std::string("Bad RakNet connection attempt result (") + std::to_string(result) + ')');
		}

		std::string addressStr = RakNet::SystemAddress(hostCstr, port).ToString();
		auto tce = pplx::task_completion_event<IConnection*>();
		_pendingConnections[addressStr] = tce;
		return pplx::task<IConnection*>(tce);
	}

	void RakNetTransport::onConnection(RakNet::Packet* rakNetPacket, RakNet::RakPeerInterface* server)
	{
		std::string sysAddr(rakNetPacket->systemAddress.ToString());
		_logger->log(LogLevel::Trace, "", sysAddr + " connected.", "");

		auto connection = createNewConnection(rakNetPacket->guid, server);
		server->DeallocatePacket(rakNetPacket);
		_handler->newConnection(connection);
		connectionOpened(dynamic_cast<IConnection*>(connection));

		connection->sendSystem((byte)MessageIDTypes::ID_CONNECTION_RESULT, [connection](bytestream* stream) {
			int64 sid = connection->id();
			*stream << sid;
		});
	}

	void RakNetTransport::onDisconnection(RakNet::Packet* packet, RakNet::RakPeerInterface* server, std::string reason)
	{
		std::string sysAddr(packet->systemAddress.ToString());
		_logger->log(LogLevel::Trace, "", sysAddr + " disconnected.", "");
		auto c = removeConnection(packet->guid);
		server->DeallocatePacket(packet);

		_handler->closeConnection(c, reason);

		connectionClosed(dynamic_cast<IConnection*>(c));
		c->connectionClosed(reason);
		delete c;
	}

	void RakNetTransport::onMessageReceived(RakNet::Packet* rakNetPacket)
	{
		auto connection = getConnection(rakNetPacket->guid);
		auto stream = new bytestream;
		stream->rdbuf()->pubsetbuf((char*)rakNetPacket->data, rakNetPacket->length);
		Packet_ptr packet(new Packet<>(connection, stream));
		auto peer = this->_peer;
		packet->cleanup += std::function<void(void)>([stream, peer, rakNetPacket]() {
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
		auto lambdaOnRequestClose = [this](RakNetConnection* c) {
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

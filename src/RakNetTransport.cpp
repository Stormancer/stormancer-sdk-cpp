#include "stormancer.h"

#if defined(_WIN32)
//
#else
#include <unistd.h>
#endif

namespace Stormancer
{
	RakNetTransport::RakNetTransport(DependencyResolver* resolver)
		: _isRunning(false),
		_dependencyResolver(resolver),
		_logger(resolver->resolve<ILogger>()),
		_scheduler(resolver->resolve<IScheduler>()),
		_name("raknet")
	{
	}

	RakNetTransport::~RakNetTransport()
	{
		stop();
	}

	void RakNetTransport::start(std::string type, IConnectionManager* handler, pplx::cancellation_token token, uint16 maxConnections, uint16 serverPort)
	{
		if (_isRunning)
		{
			throw std::runtime_error("RakNet transport is already started.");
		}

		if (handler == nullptr && serverPort > 0)
		{
			throw std::invalid_argument("Handler is null for server.");
		}

		_mutex.lock();
		_type = type;
		_handler = handler;
		initialize(serverPort, maxConnections);
		_scheduledTransportLoop = _scheduler->schedulePeriodic(15, [this]() {
			run();
		});
		token.register_callback([this]() {
			stop();
		});
		_mutex.unlock();
	}

	void RakNetTransport::initialize(uint16 maxConnections, uint16 serverPort)
	{
		try
		{
			_isRunning = true;
			_logger->log(LogLevel::Trace, "RakNetTransport::initialize", "Starting raknet transport", _type.c_str());
			_peer = RakNet::RakPeerInterface::GetInstance();
			_dependencyResolver->registerDependency(_peer);
			_socketDescriptor = (serverPort != 0 ? new RakNet::SocketDescriptor(serverPort, nullptr) : new RakNet::SocketDescriptor());
			auto startupResult = _peer->Startup(maxConnections, _socketDescriptor, 1);
			if (startupResult != RakNet::StartupResult::RAKNET_STARTED)
			{
				throw std::runtime_error(std::string("RakNet peer startup failed (RakNet::StartupResult == ") + std::to_string(startupResult) + ')');
			}
			_peer->SetMaximumIncomingConnections(maxConnections);
			_logger->log(LogLevel::Trace, "RakNetTransport::initialize", "Raknet transport started", _type.c_str());
		}
		catch (const std::exception& e)
		{
			throw std::runtime_error(std::string(e.what()) + "\nFailed to initialize the RakNet peer.");
		}
	}

	void RakNetTransport::run()
	{
		if (!_isRunning)
		{
			return;
		}

		_mutex.lock();
		if (_isRunning)
		{
			try
			{
				RakNet::Packet* rakNetPacket;
				while (rakNetPacket = _peer->Receive())
				{
					if (rakNetPacket->length == 0)
					{
						continue;
					}
					
#ifdef STORMANCER_LOG_RAKNET_PACKETS
					std::string receivedData((char*)rakNetPacket->data, rakNetPacket->length);
					auto bytes = stringifyBytesArray(receivedData, true);
					ILogger::instance()->log(LogLevel::Trace, "onMessageReceived", "data", bytes.c_str());
#endif
					
					try
					{
						byte ID = rakNetPacket->data[0];

						if (_onTransportEvent)
						{
							_onTransportEvent((void*)&ID, (void*)rakNetPacket, (void*)_peer);
						}

						switch (ID)
						{
						case (byte)DefaultMessageIDTypes::ID_CONNECTION_REQUEST_ACCEPTED:
						{
							std::string packetSystemAddressStr = rakNetPacket->systemAddress.ToString(true, ':');
							if (mapContains(_pendingConnections, packetSystemAddressStr))
							{
								auto tce = _pendingConnections[packetSystemAddressStr];

								_logger->log(LogLevel::Trace, "RakNetTransport::run", "Connection request accepted.", packetSystemAddressStr.c_str());
								auto c = onConnection(rakNetPacket, _peer);
								tce.set(c);
							}
							else
							{
								_logger->log(LogLevel::Error, "RakNetTransport::run", "Can't get the pending connection TCE.", packetSystemAddressStr.c_str());
							}
							break;
						}
						case (byte)DefaultMessageIDTypes::ID_NEW_INCOMING_CONNECTION:
						{
							_logger->log(LogLevel::Trace, "RakNetTransport::run", "Incoming connection.", rakNetPacket->systemAddress.ToString(true, ':'));
							onConnection(rakNetPacket, _peer);
							break;
						}
						case (byte)DefaultMessageIDTypes::ID_DISCONNECTION_NOTIFICATION:
						{
							_logger->log(LogLevel::Trace, "RakNetTransport::run", "Peer disconnected.", rakNetPacket->systemAddress.ToString(true, ':'));
							onDisconnection(rakNetPacket, _peer, "CLIENT_DISCONNECTED");
							break;
						}
						case (byte)DefaultMessageIDTypes::ID_CONNECTION_LOST:
						{
							_logger->log(LogLevel::Trace, "RakNetTransport::run", "Peer lost the connection.", rakNetPacket->systemAddress.ToString(true, ':'));
							onDisconnection(rakNetPacket, _peer, "CONNECTION_LOST");
							break;
						}
						case (byte)DefaultMessageIDTypes::ID_CONNECTION_ATTEMPT_FAILED:
						{
							std::string packetSystemAddressStr = rakNetPacket->systemAddress.ToString(true, ':');
							if (mapContains(_pendingConnections, packetSystemAddressStr))
							{
								auto tce = _pendingConnections[packetSystemAddressStr];
								tce.set_exception<std::exception>(std::runtime_error("Connection attempt failed."));
							}
							_peer->DeallocatePacket(rakNetPacket);
							break;
						}
						case (byte)DefaultMessageIDTypes::ID_ALREADY_CONNECTED:
						{
							_logger->log(LogLevel::Error, "RakNetTransport::run", "Peer already connected", rakNetPacket->systemAddress.ToString(true, ':'));
							_peer->DeallocatePacket(rakNetPacket);
							break;
						}
						case (byte)MessageIDTypes::ID_CONNECTION_RESULT:
						{
							int64 sid = *(int64*)(rakNetPacket->data + 1);
							_logger->log(LogLevel::Trace, "RakNetTransport::run", "Connection ID received.", std::to_string(sid).c_str());
							onConnectionIdReceived(sid);
							_peer->DeallocatePacket(rakNetPacket);
							break;
						}
						default:
						{
							onMessageReceived(rakNetPacket);
							break;
						}
						}
					}
					catch (const std::exception& ex)
					{
						_logger->log(LogLevel::Error, "RakNetTransport::run", "An error occured while handling a message.", ex.what());
					}
				}
			}
			catch (const std::exception& ex)
			{
				_logger->log(LogLevel::Error, "RakNetTransport::run", "An error occured while running the transport.", ex.what());
			}
		}
		_mutex.unlock();
	}

	void RakNetTransport::stop()
	{
		_mutex.lock();
		if (_isRunning)
		{
			_isRunning = false;
			_scheduledTransportLoop->unsubscribe();
			if (_peer)
			{
				if (_peer->IsActive())
				{
					_peer->Shutdown(1000);
				}
				RakNet::RakPeerInterface::DestroyInstance(_peer);
				_peer = nullptr;
			}
			if (_socketDescriptor)
			{
				delete _socketDescriptor;
				_socketDescriptor = nullptr;
			}
			ILogger::instance()->log(LogLevel::Trace, "RakNetTransport::run", "Stopped raknet server.", "");
		}

		if (_handler)
		{
			delete _handler;
			_handler = nullptr;
		}
		_mutex.unlock();
	}

	void RakNetTransport::onConnectionIdReceived(uint64 p)
	{
		_id = p;
	}

	pplx::task<IConnection*> RakNetTransport::connect(std::string endpoint)
	{
		std::smatch m;
		std::regex ipRegex("^(([0-9A-F]{1,4}:){7}[0-9A-F]{1,4}|(\\d{1,3}\\.){3}\\d{1,3}):(\\d{1,5})$", std::regex_constants::ECMAScript | std::regex_constants::icase);
		bool res = std::regex_search(endpoint, m, ipRegex);
		if (!res || m.length() < 5)
		{
			throw std::invalid_argument("Bad scene endpoint (" + endpoint + ')');
		}

		_host = m.str(1);
		auto hostCstr = _host.c_str();
		_port = (uint16)std::atoi(m.str(4).c_str());
		if (_port == 0)
		{
			throw std::runtime_error("Scene endpoint port should not be 0 (" + endpoint + ')');
		}

		_mutex.lock();
		bool transportNotStarted = (_peer == nullptr || !_peer->IsActive());
		_mutex.unlock();
		if (transportNotStarted)
		{
			throw std::runtime_error("Transport not started. Check you started it.");
		}

		_mutex.lock();
		auto result = _peer->Connect(hostCstr, _port, "", 0);
		_mutex.unlock();
		if (result != RakNet::ConnectionAttemptResult::CONNECTION_ATTEMPT_STARTED)
		{
			throw std::runtime_error(std::string("Bad RakNet connection attempt result (") + std::to_string(result) + ')');
		}

		std::string addressStr = RakNet::SystemAddress(hostCstr, _port).ToString(true, ':');
		auto tce = pplx::task_completion_event<IConnection*>();
		_pendingConnections[addressStr] = tce;
		return pplx::task<IConnection*>(tce);
	}

	RakNetConnection* RakNetTransport::onConnection(RakNet::Packet* packet, RakNet::RakPeerInterface* server)
	{
		_logger->log(LogLevel::Trace, "RakNetTransport::onConnection", (std::string(packet->systemAddress.ToString(true, ':')) + " connected.").c_str(), "");

		auto c = createNewConnection(packet->guid, _peer);

		c->setConnectionState(ConnectionState::Connected);

		_handler->newConnection(c);

		_connectionOpened(dynamic_cast<IConnection*>(c));

		c->sendSystem((byte)MessageIDTypes::ID_CONNECTION_RESULT, [c](bytestream* stream) {
			int64 sid = c->id();
			*stream << sid;
		});

		server->DeallocatePacket(packet);

		return c;
	}

	void RakNetTransport::onDisconnection(RakNet::Packet* packet, RakNet::RakPeerInterface* server, std::string reason)
	{
		_logger->log(LogLevel::Trace, "RakNetTransport::onDisconnection", (std::string(packet->systemAddress.ToString(true, ':')) + " disconnected.").c_str(), "");

		auto c = removeConnection(packet->guid);

		_handler->closeConnection(c, reason);

		_connectionClosed(dynamic_cast<IConnection*>(c));

		c->setConnectionState(ConnectionState::Disconnected);

		c->_onClose(reason.c_str());

		server->DeallocatePacket(packet);

		delete c;
	}

	void RakNetTransport::onMessageReceived(RakNet::Packet* rakNetPacket)
	{
#if defined(STORMANCER_LOG_PACKETS) && !defined(STORMANCER_LOG_RAKNET_PACKETS)
		std::string receivedData((char*)rakNetPacket->data, rakNetPacket->length);
		auto bytes = stringifyBytesArray(receivedData, true);
		ILogger::instance()->log(LogLevel::Trace, "RakNetTransport::onMessageReceived", "data", bytes.c_str());
#endif

		auto connection = getConnection(rakNetPacket->guid);
		auto stream = new bytestream;
		stream->rdbuf()->pubsetbuf((char*)rakNetPacket->data, rakNetPacket->length);
		Packet_ptr packet(new Packet<>(connection, stream), deleter<Packet<>>());
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

		_onPacketReceived(packet);
	}

	RakNetConnection* RakNetTransport::getConnection(RakNet::RakNetGUID guid)
	{
		return _connections[guid.g];
	}

	RakNetConnection* RakNetTransport::createNewConnection(RakNet::RakNetGUID raknetGuid, RakNet::RakPeerInterface* peer)
	{
		int64 cid = _handler->generateNewConnectionId();
		auto onClose = [this](RakNetConnection* c) {
			onRequestClose(c);
		};
		auto c = new RakNetConnection(raknetGuid, cid, peer, onClose);
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

	bool RakNetTransport::isRunning() const
	{
		return _isRunning;
	}

	std::string RakNetTransport::name() const
	{
		return _name;
	}

	uint64 RakNetTransport::id() const
	{
		return _id;
	}

	DependencyResolver* RakNetTransport::dependencyResolver() const
	{
		return _dependencyResolver;
	}

	void RakNetTransport::onPacketReceived(std::function<void(Packet_ptr)> callback)
	{
		_onPacketReceived += callback;
	}

	const char* RakNetTransport::host() const
	{
		return _host.c_str();
	}

	uint16 RakNetTransport::port() const
	{
		return _port;
	}

	void RakNetTransport::onTransportEvent(std::function<void(void*, void*, void*)> callback)
	{
		_onTransportEvent = callback;
	}
};

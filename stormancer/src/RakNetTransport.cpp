#include "stormancer.h"

#if defined(_WIN32)
//
#else
#include <unistd.h>
#endif

namespace Stormancer
{
	RakNetTransport::RakNetTransport(DependencyResolver* resolver)
		: _dependencyResolver(resolver)
		, _logger(resolver->resolve<ILogger>())
		, _scheduler(resolver->resolve<IScheduler>())
		, _name("raknet")
	{
	}

	RakNetTransport::~RakNetTransport()
	{
		ILogger::instance()->log(LogLevel::Trace, "RakNetTransport::~RakNetTransport", "Deleting the RakNet transport...");
		stop();
		ILogger::instance()->log(LogLevel::Trace, "RakNetTransport::~RakNetTransport", "RakNet transport deleted");
	}

	void RakNetTransport::start(std::string type, IConnectionManager* handler, pplx::cancellation_token token, uint16 maxConnections, uint16 serverPort)
	{
		std::lock_guard<std::mutex> lock(_mutex);

		if (isRunning())
		{
			throw std::runtime_error("RakNet transport is already started.");
		}

		if (handler == nullptr && serverPort > 0)
		{
			throw std::invalid_argument("Handler is null for server.");
		}

		_type = type;
		_handler = handler;
		initialize(serverPort, maxConnections);
		_scheduledTransportLoop = _scheduler->schedulePeriodic(15, [this]() {
			std::lock_guard<std::mutex> lock(_mutex);
			if (_scheduledTransportLoop && _scheduledTransportLoop->subscribed())
			{
				run();
			}
		});
		token.register_callback([this]() {
			stop();
		});
	}

	void RakNetTransport::initialize(uint16 maxConnections, uint16 serverPort)
	{
		try
		{
			_logger->log(LogLevel::Trace, "RakNetTransport::initialize", "Starting raknet transport");
			_peer = std::shared_ptr<RakNet::RakPeerInterface>(RakNet::RakPeerInterface::GetInstance(), [](RakNet::RakPeerInterface* peer) {
				RakNet::RakPeerInterface::DestroyInstance(peer);
			});
			_dependencyResolver->registerDependency(_peer);
			_socketDescriptor = (serverPort != 0 ? new RakNet::SocketDescriptor(serverPort, nullptr) : new RakNet::SocketDescriptor());
			auto startupResult = _peer->Startup(maxConnections, _socketDescriptor, 1);
			if (startupResult != RakNet::StartupResult::RAKNET_STARTED)
			{
				throw std::runtime_error(std::string("RakNet peer startup failed (RakNet::StartupResult == ") + std::to_string(startupResult) + ')');
			}
			_peer->SetMaximumIncomingConnections(maxConnections);
			_logger->log(LogLevel::Trace, "RakNetTransport::initialize", "Raknet transport started");
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
			RakNet::Packet* rakNetPacket = nullptr;
			while (rakNetPacket = _peer->Receive())
			{
				if (rakNetPacket->length == 0)
				{
					continue;
				}

				byte ID = rakNetPacket->data[0];

#ifdef STORMANCER_LOG_RAKNET_PACKETS
				std::string receivedData((char*)rakNetPacket->data, rakNetPacket->length);
				auto bytes = stringifyBytesArray(receivedData, true);
				ILogger::instance()->log(LogLevel::Trace, "RakNetTransport::run", "RakNet packet received", bytes.c_str());
#endif

				static bool _serverConnected = false;
				std::string packetSystemAddressStr = rakNetPacket->systemAddress.ToString(true, ':');
				if (_serverConnected && !mapContains(_pendingConnections, packetSystemAddressStr))
				{

					if (_onTransportEvent)
					{
						_onTransportEvent((void*)&ID, (void*)rakNetPacket, (void*)_peer.get());
					}
					continue;
				}

				try
				{
					switch (ID)
					{
					case DefaultMessageIDTypes::ID_CONNECTION_REQUEST_ACCEPTED:
					{
						if (!_serverConnected)
						{
							_serverConnected = true;
							_serverRakNetGUID = rakNetPacket->guid;
						}

						std::string packetSystemAddressStr = rakNetPacket->systemAddress.ToString(true, ':');
						if (mapContains(_pendingConnections, packetSystemAddressStr))
						{
							auto tce = _pendingConnections[packetSystemAddressStr];

							_logger->log(LogLevel::Trace, "RakNetTransport::run", "Connection request accepted. "+ packetSystemAddressStr);
							auto c = onConnection(rakNetPacket);
							tce.set(c);
						}
						else
						{
							_logger->log(LogLevel::Error, "RakNetTransport::run", "A connection attempt response was received, without corresponding pending connection :"+ packetSystemAddressStr);
						}
						break;
					}
					case DefaultMessageIDTypes::ID_NEW_INCOMING_CONNECTION:
					{
						_logger->log(LogLevel::Trace, "RakNetTransport::run", "Incoming connection. "+std::string( rakNetPacket->systemAddress.ToString(true, ':')));
						onConnection(rakNetPacket);
						break;
					}
					case DefaultMessageIDTypes::ID_DISCONNECTION_NOTIFICATION:
					{
						_logger->log(LogLevel::Trace, "RakNetTransport::run", "Peer disconnected : "+std::string( rakNetPacket->systemAddress.ToString(true, ':')));
						onDisconnection(rakNetPacket, "CLIENT_DISCONNECTED");
						break;
					}
					case DefaultMessageIDTypes::ID_CONNECTION_LOST:
					{
						_logger->log(LogLevel::Trace, "RakNetTransport::run", "Peer lost the connection to "+std::string(rakNetPacket->systemAddress.ToString(true, ':')));
						onDisconnection(rakNetPacket, "CLIENT_CONNECTION_LOST");
						break;
					}
					case DefaultMessageIDTypes::ID_CONNECTION_ATTEMPT_FAILED:
					{
						std::string packetSystemAddressStr = rakNetPacket->systemAddress.ToString(true, ':');
						if (mapContains(_pendingConnections, packetSystemAddressStr))
						{
							auto tce = _pendingConnections[packetSystemAddressStr];
							tce.set_exception<std::exception>(std::runtime_error("Connection attempt failed."));
						}
						break;
					}
					case DefaultMessageIDTypes::ID_ALREADY_CONNECTED:
					{
						_logger->log(LogLevel::Error, "RakNetTransport::run", "Peer already connected "+ std::string(rakNetPacket->systemAddress.ToString(true, ':')));
						break;
					}
					case (byte)MessageIDTypes::ID_CONNECTION_RESULT:
					{
						int64 sid = *(int64*)(rakNetPacket->data + 1);
						_logger->log(LogLevel::Trace, "RakNetTransport::run", "Connection ID received : "+ std::to_string(sid));
						onConnectionIdReceived(sid);
						break;
					}
					default:
					{
						onMessageReceived(rakNetPacket);
						rakNetPacket = nullptr;
						break;
					}
					}
					_peer->DeallocatePacket(rakNetPacket);
				}
				catch (const std::exception& ex)
				{
					_logger->log(LogLevel::Error, "RakNetTransport::run", "An error occured while handling a message :"+std::string(ex.what()));
				}
			}
		}
		catch (const std::exception& ex)
		{
			_logger->log(LogLevel::Error, "RakNetTransport::run", "An error occured while running the transport :" + std::string( ex.what()));
		}
	}

	void RakNetTransport::stop()
	{
		ILogger::instance()->log(LogLevel::Trace, "RakNetTransport::stop", "Stopping RakNet...");

		ILogger::instance()->log(LogLevel::Trace, "RakNetTransport::stop", "Waiting for RakNet loop to stop");
		std::lock_guard<std::mutex> lg(_mutex);
		ILogger::instance()->log(LogLevel::Trace, "RakNetTransport::stop", "RakNet loop stopped");

		if (_scheduledTransportLoop && _scheduledTransportLoop->subscribed())
		{
			_scheduledTransportLoop->unsubscribe();
			_scheduledTransportLoop->destroy();
			_scheduledTransportLoop = nullptr;

			if (_peer)
			{
				if (_peer->IsActive())
				{
					_peer->Shutdown(1000);
				}
				
			}
			if (_socketDescriptor)
			{
				delete _socketDescriptor;
				_socketDescriptor = nullptr;
			}

			if (_handler)
			{
				delete _handler;
				_handler = nullptr;
			}
		}

		ILogger::instance()->log(LogLevel::Trace, "RakNetTransport::stop", "RakNet stopped");
	}

	void RakNetTransport::onConnectionIdReceived(uint64 p)
	{
		_id = p;
	}

	pplx::task<IConnection*> RakNetTransport::connect(std::string endpoint)
	{
		std::lock_guard<std::mutex> lock(_mutex);

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

		if (_peer == nullptr || !_peer->IsActive())
		{
			throw std::runtime_error("Transport not started. Check you started it.");
		}

		auto result = _peer->Connect(hostCstr, _port, "", 0);
		if (result != RakNet::ConnectionAttemptResult::CONNECTION_ATTEMPT_STARTED)
		{
			throw std::runtime_error(std::string("Bad RakNet connection attempt result (") + std::to_string(result) + ')');
		}

		std::string addressStr = RakNet::SystemAddress(hostCstr, _port).ToString(true, ':');
		auto tce = pplx::task_completion_event<IConnection*>();
		_pendingConnections[addressStr] = tce;
		return pplx::task<IConnection*>(tce);
	}

	RakNetConnection* RakNetTransport::onConnection(RakNet::Packet* packet)
	{
		_logger->log(LogLevel::Trace, "RakNetTransport::onConnection", std::string(packet->systemAddress.ToString(true, ':')) + " connected.");

		auto c = createNewConnection(packet->guid, _peer.get());

		_handler->newConnection(c);

		c->sendSystem((byte)MessageIDTypes::ID_CONNECTION_RESULT, [c](bytestream* stream) {
			int64 sid = c->id();
			*stream << sid;
		});

		pplx::task<void>([c]() {
			// Start this asynchronously because we locked the mutex in run and the user can do something that tries to lock again this mutex
			c->setConnectionState(ConnectionState::Connected);
		});

		return c;
	}

	void RakNetTransport::onDisconnection(RakNet::Packet* packet, std::string reason)
	{
		_logger->log(LogLevel::Trace, "RakNetTransport::onDisconnection", std::string(packet->systemAddress.ToString(true, ':')) + " disconnected : "+reason);

		auto c = removeConnection(packet->guid);

		_handler->closeConnection(c, reason);

		c->_closeAction(reason);

		pplx::task<void>([c]() {
			// Start this asynchronously because we locked the mutex in run and the user can do something that tries to lock again this mutex
			c->setConnectionState(ConnectionState::Disconnected);
			delete c;
		});
	}

	void RakNetTransport::onMessageReceived(RakNet::Packet* rakNetPacket)
	{
#if defined(STORMANCER_LOG_PACKETS) && !defined(STORMANCER_LOG_RAKNET_PACKETS)
		std::string receivedData((char*)rakNetPacket->data, rakNetPacket->length);
		auto bytes = stringifyBytesArray(receivedData, true);
		ILogger::instance()->log(LogLevel::Trace, "RakNetTransport::onMessageReceived", "Packet received", bytes.c_str());
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
		auto c = new RakNetConnection(raknetGuid, cid, peer);
		c->onClose([this, c](std::string reason) {
			ILogger::instance()->log(LogLevel::Trace, "RakNetTransport::Connection::onClose", "closing "+ std::string(c->guid().ToString()));
			onRequestClose(c);
		});
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
		if (_peer)
		{
			_peer->CloseConnection(c->guid(), true);
		}
	}

	bool RakNetTransport::isRunning() const
	{
		return (_scheduledTransportLoop && _scheduledTransportLoop->subscribed());
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

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
			_logger->log(LogLevel::Info, "RakNetTransport::initialize", std::string("Starting raknet transport ") + _type, "");
			_peer = RakNet::RakPeerInterface::GetInstance();
			_socketDescriptor = (serverPort != 0 ? new RakNet::SocketDescriptor(serverPort, nullptr) : new RakNet::SocketDescriptor());
			auto startupResult = _peer->Startup(maxConnections, _socketDescriptor, 1);
			if (startupResult != RakNet::StartupResult::RAKNET_STARTED)
			{
				throw std::runtime_error(std::string("RakNet peer startup failed (RakNet::StartupResult == ") + std::to_string(startupResult) + ')');
			}
			_peer->SetMaximumIncomingConnections(maxConnections);
			_logger->log(LogLevel::Info, "RakNetTransport::initialize", std::string("Raknet transport started ") + _type, "");
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
			RakNet::Packet* packet;
			while ((packet = _peer->Receive()) != nullptr)
			{
				if (packet->length == 0)
				{
					continue;
				}

				ILogger::instance()->log(std::string("RECEIVED: ") + stringifyBytesArray(std::string(packet->data, packet->data + packet->length)));

				try
				{
					switch (packet->data[0])
					{
					case (byte)DefaultMessageIDTypes::ID_CONNECTION_REQUEST_ACCEPTED:
					{
						std::string packetSystemAddressStr = packet->systemAddress.ToString();
						if (mapContains(_pendingConnections, packetSystemAddressStr))
						{
							auto tce = _pendingConnections[packetSystemAddressStr];

							_logger->log(LogLevel::Debug, "RakNetTransport::run", "Connection request to accepted.", packetSystemAddressStr);
							onConnection(packet, _peer);

							if (mapContains(_connections, packet->guid.g))
							{
								auto c = _connections[packet->guid.g];
								tce.set(c);
							}
							else
							{
								_logger->log(LogLevel::Error, "RakNetTransport::run", "Can't get the peer connection.", packetSystemAddressStr);
							}
						}
						else
						{
							_logger->log(LogLevel::Error, "RakNetTransport::run", "Can't get the pending connection TCE.", packetSystemAddressStr);
						}
						break;
					}
					case (byte)DefaultMessageIDTypes::ID_NEW_INCOMING_CONNECTION:
					{
						_logger->log(LogLevel::Trace, "RakNetTransport::run", "Incoming connection.", packet->systemAddress.ToString());
						onConnection(packet, _peer);
						break;
					}
					case (byte)DefaultMessageIDTypes::ID_DISCONNECTION_NOTIFICATION:
					{
						_logger->log(LogLevel::Trace, "RakNetTransport::run", "Peer disconnected.", packet->systemAddress.ToString());
						onDisconnection(packet, _peer, "CLIENT_DISCONNECTED");
						break;
					}
					case (byte)DefaultMessageIDTypes::ID_CONNECTION_LOST:
					{
						_logger->log(LogLevel::Trace, "RakNetTransport::run", "Peer lost the connection.", packet->systemAddress.ToString());
						onDisconnection(packet, _peer, "CONNECTION_LOST");
						break;
					}
					case (byte)DefaultMessageIDTypes::ID_CONNECTION_ATTEMPT_FAILED:
					{
						std::string packetSystemAddressStr = packet->systemAddress.ToString();
						if (mapContains(_pendingConnections, packetSystemAddressStr))
						{
							auto tce = _pendingConnections[packetSystemAddressStr];
							tce.set_exception<std::exception>(std::runtime_error("Connection attempt failed."));
						}
						break;
					}
					case (byte)DefaultMessageIDTypes::ID_ALREADY_CONNECTED:
						throw std::logic_error(std::string("Peer already connected (") + packet->systemAddress.ToString() + ")");
						break;
					case (byte)MessageIDTypes::ID_CONNECTION_RESULT:
					{
						int64 sid = *(int64*)(packet->data + 1);
						_logger->log(LogLevel::Debug, "RakNetTransport::run", "Connection ID received.", std::to_string(sid));
						onConnectionIdReceived(sid);
						break;
					}
					default:
					{
						onMessageReceived(packet);
						break;
					}
					}
				}
				catch (std::exception e)
				{
					_logger->log(LogLevel::Error, "RakNetTransport::run", "An error occured while handling a message.", e.what());
				}
			}
		}
		catch (std::exception e)
		{
			_logger->log(LogLevel::Error, "RakNetTransport::run", "An error occured while running the transport.", e.what());
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
			ILogger::instance()->log(LogLevel::Info, "RakNetTransport::run", "Stopped raknet server.", "");
		}
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

		auto host = m.str(1);
		auto hostCstr = host.c_str();
		uint16 port = (uint16)std::atoi(m.str(4).c_str());
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

	void RakNetTransport::onConnection(RakNet::Packet* packet, RakNet::RakPeerInterface* server)
	{
		_logger->log(LogLevel::Trace, "RakNetTransport::onConnection", std::string(packet->systemAddress.ToString()) + " connected.", "");

		auto c = createNewConnection(packet->guid, _peer);

		server->DeallocatePacket(packet);

		_handler->newConnection(c);

		connectionOpened(dynamic_cast<IConnection*>(c));

		c->sendSystem((byte)MessageIDTypes::ID_CONNECTION_RESULT, [c](bytestream* stream) {
			int64 sid = c->id();
			*stream << sid;
		});
	}

	void RakNetTransport::onDisconnection(RakNet::Packet* packet, RakNet::RakPeerInterface* server, std::string reason)
	{
		_logger->log(LogLevel::Trace, "RakNetTransport::onDisconnection", std::string(packet->systemAddress.ToString()) + " disconnected.", "");

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

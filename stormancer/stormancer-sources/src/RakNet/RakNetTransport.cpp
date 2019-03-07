#include "stormancer/stdafx.h"
#include "GetTime.h"
#include "BitStream.h"
#include "stormancer/RakNet/RakNetTransport.h"
#include "stormancer/IScheduler.h"
#include "stormancer/MessageIDTypes.h"
#include "MessageIdentifiers.h"
#include "stormancer/SafeCapture.h"
#include "PacketFileLogger.h"
#include "stormancer/Debug/StackWalker.h"
#include "stormancer/utilities/taskUtilities.h"
#include "stormancer/SceneImpl.h"

#if defined(_WIN32)
//
#else
#include <unistd.h>
#endif

namespace Stormancer
{
	RakNetTransport::RakNetTransport(std::weak_ptr<DependencyResolver> resolver)
		: _dependencyResolver(resolver)
	{
		auto dependencyResolver = _dependencyResolver.lock();
		if (!dependencyResolver)
		{
			throw std::runtime_error("Dependency resolver is invalid");
		}

		_logger = dependencyResolver->resolve<ILogger>();
		_scheduler = dependencyResolver->resolve<IScheduler>();
	}

	RakNetTransport::~RakNetTransport()
	{
		_logger->log(LogLevel::Trace, "RakNetTransport", "Deleting RakNet transport...");

		if (_isRunning)
		{
			stop();
		}

		_logger->log(LogLevel::Trace, "RakNetTransport", "RakNet transport deleted");
	}

	void RakNetTransport::start(std::string type, std::shared_ptr<IConnectionManager> handler, pplx::cancellation_token ct, uint16 serverPort, uint16 maxConnections)
	{
		_logger->log(LogLevel::Trace, "RakNetTransport", "Starting RakNet transport...");

		if (!compareExchange(_mutex, _isRunning, false, true))
		{
			return;
		}

		if (handler == nullptr && maxConnections > 0)
		{
			throw std::invalid_argument("Handler is null for server.");
		}

		_type = type;
		_handler = handler;
		initialize(maxConnections, serverPort);

		auto wTransport = STRM_WEAK_FROM_THIS();

		_scheduler->schedulePeriodic(15, [wTransport]()
		{
			auto transport = LockOrThrow(wTransport);
			std::lock_guard<std::mutex> lock(transport->_mutex);
			if (transport->_isRunning)
			{
				transport->run();
			}
		}, ct);

		ct.register_callback([wTransport]()
		{
			auto transport = LockOrThrow(wTransport);
			transport->stop();
		});

		_logger->log(LogLevel::Trace, "RakNetTransport", "RakNet transport started");
	}

	void RakNetTransport::initialize(uint16 maxConnections, uint16 serverPort)
	{
		try
		{
			_logger->log(LogLevel::Trace, "RakNetTransport", "Initializing raknet transport", _type.c_str());

#ifdef STORMANCER_PACKETFILELOGGER
			RakNet::PacketFileLogger* rakNetLogger = nullptr;
			rakNetLogger = new RakNet::PacketFileLogger();
			rakNetLogger->StartLog("packetLogs");
#endif

			auto wTransport = STRM_WEAK_FROM_THIS();
			_peer = std::shared_ptr<RakNet::RakPeerInterface>(RakNet::RakPeerInterface::GetInstance(), [wTransport](RakNet::RakPeerInterface* peer)
			{
				if (auto transport = wTransport.lock())
				{
					transport->_logger->log(LogLevel::Trace, "RakNetTransport", "Deleting RakPeerInterface...");
				}
				RakNet::RakPeerInterface::DestroyInstance(peer);
#ifdef STORMANCER_PACKETFILELOGGER
				if (rakNetLogger)
				{
					delete rakNetLogger;
				}
#endif
			});

#ifdef STORMANCER_PACKETFILELOGGER
			_peer->AttachPlugin(rakNetLogger);
#endif
			auto dependencyResolver = _dependencyResolver.lock();
			if (!dependencyResolver)
			{
				throw std::runtime_error("Dependency resolver is invalid");
			}

			dependencyResolver->registerDependency(_peer);

			if (serverPort != 0)
			{
				_socketDescriptor = std::make_shared<RakNet::SocketDescriptor>(serverPort, (const char*)nullptr);
			}
			else
			{
				_socketDescriptor = std::make_shared<RakNet::SocketDescriptor>();
			}
			_socketDescriptor->socketFamily = AF_INET;
			DataStructures::List<RakNet::SocketDescriptor> socketDescriptorsList;
			socketDescriptorsList.Push(*_socketDescriptor, _FILE_AND_LINE_);
			auto startupResult = _peer->Startup(maxConnections, socketDescriptorsList, 1);
			if (startupResult != RakNet::StartupResult::RAKNET_STARTED)
			{
				throw std::runtime_error((std::string("RakNet peer startup failed (RakNet::StartupResult == ") + std::to_string(startupResult) + ')').c_str());
			}
			_peer->SetMaximumIncomingConnections(maxConnections);
			_logger->log(LogLevel::Trace, "RakNetTransport", "Raknet transport initialized", _type.c_str());
		}
		catch (const std::exception& ex)
		{
			throw std::runtime_error((std::string() + "Failed to initialize the RakNet peer (" + ex.what() + ")").c_str());
		}
	}

	void RakNetTransport::run()
	{
		try
		{
			RakNet::Packet* rakNetPacket = nullptr;
			while ((rakNetPacket = _peer->Receive()) != nullptr)
			{
				if (rakNetPacket->length == 0)
				{
					assert(false);
					if (_peer->IsActive())
					{
						_peer->DeallocatePacket(rakNetPacket);
					}
					continue;
				}

				byte ID = rakNetPacket->data[0];
				//_logger->log(LogLevel::Trace, "RakNetTransport", "RakNet packet received ", std::to_string(ID));
#ifdef STORMANCER_LOG_RAKNET_PACKETS
				std::vector<byte> receivedData(rakNetPacket->data, rakNetPacket->data + rakNetPacket->length);
				auto bytes = stringifyBytesArray(receivedData, true);
				_logger->log(LogLevel::Trace, "RakNetTransport", "RakNet packet received", bytes.c_str());
#endif

				try
				{
					switch (ID)
					{
						// RakNet messages types
					case DefaultMessageIDTypes::ID_CONNECTION_REQUEST_ACCEPTED:
					{
						std::lock_guard<std::mutex> lg(_pendingConnection_mtx);

						std::string packetSystemAddressStr = rakNetPacket->systemAddress.ToString(true, ':');
						if (!_pendingConnections.empty())
						{
							auto rq = _pendingConnections.front();
							if (rq.cancellationToken.is_canceled())
							{
								_peer->CloseConnection(rakNetPacket->guid, false);
							}
							else if (rq.isP2P())// the connection is a P2P connection, 
							{
								auto parentConnection = _handler->getConnection(rq.parentId);

								RakNet::BitStream data;
								data.Write((byte)MessageIDTypes::ID_ADVERTISE_PEERID);
								data.Write(parentConnection->id());
								data.Write(rq.parentId.c_str());
								data.Write(rq.id.c_str());
								data.Write(true);
								_peer->Send(&data, PacketPriority::MEDIUM_PRIORITY, PacketReliability::RELIABLE, 0, rakNetPacket->guid, false);


							}

						}
						else
						{
							_logger->log(LogLevel::Error, "RakNetTransport", "Can't get the pending connection TCE", packetSystemAddressStr.c_str());
						}
						break;
					}
					case DefaultMessageIDTypes::ID_NO_FREE_INCOMING_CONNECTIONS:
						_logger->log(LogLevel::Trace, "RakNetTransport", "Connection failed because too many concurrent connections.");
					case DefaultMessageIDTypes::ID_INCOMPATIBLE_PROTOCOL_VERSION:
					case DefaultMessageIDTypes::ID_CONNECTION_ATTEMPT_FAILED:
					{
						std::string packetSystemAddressStr = rakNetPacket->systemAddress.ToString(true, ':');

						//_peer->CloseConnection(rakNetPacket->guid, false);

						std::lock_guard<std::mutex> lg(_pendingConnection_mtx);

						if (!_pendingConnections.empty())
						{
							auto rq = _pendingConnections.front();
							_pendingConnections.pop();
							if (!rq.cancellationToken.is_canceled())
							{
								_logger->log(LogLevel::Trace, "RakNetTransport", "Connection request failed", packetSystemAddressStr.c_str());
								rq.tce.set_exception(std::runtime_error("Connection attempt failed"));
							}
							startNextPendingConnections();
						}
						else
						{
							_logger->log(LogLevel::Error, "RakNetTransport", "Can't get the pending connection TCE", packetSystemAddressStr.c_str());
						}
						break;
					}
					case DefaultMessageIDTypes::ID_ALREADY_CONNECTED:
					{
						_logger->log(LogLevel::Error, "RakNetTransport", "Peer already connected", rakNetPacket->systemAddress.ToString(true, ':'));
						break;
					}
					case DefaultMessageIDTypes::ID_NEW_INCOMING_CONNECTION:
					{
						/*_logger->log(LogLevel::Trace, "RakNetTransport", "Incoming connection", rakNetPacket->systemAddress.ToString(true, ':'));
						RakNet::BitStream data;
						data.Write((char)MessageIDTypes::ID_ADVERTISE_PEERID);
						data.Write(_id);
						data.Write(true);
						_peer->Send(&data, PacketPriority::MEDIUM_PRIORITY, PacketReliability::RELIABLE, 0, rakNetPacket->guid, false);*/
						break;
					}
					case DefaultMessageIDTypes::ID_DISCONNECTION_NOTIFICATION:
					{
						_logger->log(LogLevel::Trace, "RakNetTransport", "Remote peer disconnected from this peer", rakNetPacket->systemAddress.ToString(true, ':'));
						onDisconnection(rakNetPacket->guid.g, "CLIENT_DISCONNECTED");
						break;
					}
					case DefaultMessageIDTypes::ID_CONNECTION_LOST:
					{
						_logger->log(LogLevel::Trace, "RakNetTransport", "Peer lost the connection", rakNetPacket->systemAddress.ToString(true, ':'));
						onDisconnection(rakNetPacket->guid.g, "CLIENT_CONNECTION_LOST");
						break;
					}
					case DefaultMessageIDTypes::ID_CONNECTION_BANNED:
					{
						_logger->log(LogLevel::Trace, "RakNetTransport", "We are banned from the system we attempted to connect to", rakNetPacket->systemAddress.ToString(true, ':'));
						break;
					}
					case DefaultMessageIDTypes::ID_INVALID_PASSWORD:
					{
						_logger->log(LogLevel::Trace, "RakNetTransport", "The remote system is using a password and has refused our connection because we did not set the correct password", rakNetPacket->systemAddress.ToString(true, ':'));
						break;
					}
					case DefaultMessageIDTypes::ID_IP_RECENTLY_CONNECTED:
					{
						_logger->log(LogLevel::Trace, "RakNetTransport", "this IP address connected recently, and can't connect again as a security measure", rakNetPacket->systemAddress.ToString(true, ':'));
						break;
					}
					case DefaultMessageIDTypes::ID_UNCONNECTED_PONG:
					{
						auto address = std::string(rakNetPacket->systemAddress.ToString(true, ':'));
						_logger->log(LogLevel::Debug, "RakNetTransport", "Received pong message.", address);
						RakNet::BitStream data(rakNetPacket->data + 1, rakNetPacket->length - 1, false);
						RakNet::TimeMS sentOn;

						data.Read<RakNet::TimeMS>(sentOn);

						auto it = _pendingPings.find(address);

						if (it != _pendingPings.end())
						{
							it->second.set(RakNet::GetTimeMS() - sentOn);
						}
						break;
					}
					case DefaultMessageIDTypes::ID_UNCONNECTED_PING_OPEN_CONNECTIONS:
					{
						break;
					}
					case DefaultMessageIDTypes::ID_ADVERTISE_SYSTEM:
					{
						_logger->log(LogLevel::Trace, "RakNetTransport", "Inform a remote system of our IP/Port", rakNetPacket->systemAddress.ToString(true, ':'));
						break;
					}
					// Stormancer messages types
					case (byte)MessageIDTypes::ID_CONNECTION_RESULT:
					{
						std::lock_guard<std::mutex> lg(_pendingConnection_mtx);

						std::string packetSystemAddressStr = rakNetPacket->systemAddress.ToString(true, ':');
						if (!_pendingConnections.empty())
						{
							auto rq = _pendingConnections.front();
							_pendingConnections.pop();

							int64 sid;
							std::memcpy(&sid, (rakNetPacket->data + 1), sizeof(sid));

							_logger->log(LogLevel::Trace, "RakNetTransport", "Successfully connected to ", rakNetPacket->systemAddress.ToString());

							auto connection = onConnection(rakNetPacket->systemAddress, rakNetPacket->guid, sid, rq);
							startNextPendingConnections();
						}
						break;
					}
					case (byte)MessageIDTypes::ID_CLOSE_REASON:
					{
						if (auto connection = getConnection(rakNetPacket->guid.g))
						{
							Serializer serializer;
							connection->_closeReason = serializer.deserializeOne<std::string>((byte*)(rakNetPacket->data + 1), (rakNetPacket->length - 1));
						}
						break;
					}
					case (byte)DefaultMessageIDTypes::ID_UNCONNECTED_PING:
						break;
					case (byte)MessageIDTypes::ID_ADVERTISE_PEERID:
					{
						std::lock_guard<std::mutex> lg(_pendingConnection_mtx);
						char* buffer = new char[512];
						std::string parentId;
						std::string id;
						bool requestResponse = false;
						uint64 remotePeerId = 0;
						std::string packetSystemAddressStr = rakNetPacket->systemAddress.ToString(true, ':');

						{


							RakNet::BitStream data(rakNetPacket->data + 1, rakNetPacket->length - 1, false);
							data.Read(remotePeerId);

							data.Read(buffer);
							parentId = buffer;
							delete[] buffer;
							buffer = new char[512];
							data.Read(buffer);
							id = buffer;
							delete[] buffer;
							data.Read(requestResponse);
						}

						if (!requestResponse)
						{
							auto rq = _pendingConnections.front();
							_pendingConnections.pop();
							if (rq.cancellationToken.is_canceled())
							{
								_peer->CloseConnection(rakNetPacket->guid, false);
							}
							else
							{
								_logger->log(LogLevel::Trace, "RakNetTransport", "Connection request accepted", packetSystemAddressStr.c_str());
								auto connection = onConnection(rakNetPacket->systemAddress, rakNetPacket->guid, (uint64)remotePeerId, rq);
							}
							startNextPendingConnections();
						}
						else
						{
							auto parentConnection = _handler->getConnection(std::string(parentId));

							RakNet::BitStream data;
							data.Write((byte)MessageIDTypes::ID_ADVERTISE_PEERID);
							data.Write(parentConnection->id());
							data.Write(parentId);
							data.Write(id);
							data.Write(false);

							_peer->Send(&data, PacketPriority::MEDIUM_PRIORITY, PacketReliability::RELIABLE, 0, rakNetPacket->guid, false);


							onConnection(rakNetPacket->systemAddress, rakNetPacket->guid, (uint64)remotePeerId, id);
						}
						break;
					}
					case DefaultMessageIDTypes::ID_TIMESTAMP:
					case (byte)MessageIDTypes::ID_SYSTEM_REQUEST:
					case (byte)MessageIDTypes::ID_REQUEST_RESPONSE_COMPLETE:
					case (byte)MessageIDTypes::ID_REQUEST_RESPONSE_ERROR:
					case (byte)MessageIDTypes::ID_REQUEST_RESPONSE_MSG:
					case (byte)MessageIDTypes::ID_SCENES:
					default:
					{
						if (ID >= DefaultMessageIDTypes::ID_USER_PACKET_ENUM || (ID >= 58 && ID <= 59))
						{
							onMessageReceived(rakNetPacket);
						}
						else
						{
							_logger->log(LogLevel::Trace, "RakNetTransport", "Unprocessed RakNet message ID", std::to_string(ID));
						}
					}
					}
					if (rakNetPacket && _peer->IsActive())
					{
						_peer->DeallocatePacket(rakNetPacket);
						rakNetPacket = nullptr;
					}
				}
				catch (const PointerDeletedException&)
				{
				}
				catch (const std::exception& ex)
				{
					
					_logger->log(LogLevel::Error, "RakNetTransport", "An error occured while handling a message", ex.what());
				}
			}
		}
		catch (const std::exception& ex)
		{
			_logger->log(LogLevel::Error, "RakNetTransport", "An error occured while running the transport", ex.what());
		}
	}

	void RakNetTransport::stop()
	{
		_logger->log(LogLevel::Trace, "RakNetTransport", "Stopping RakNet transport...");

		if (!compareExchange(_mutex, _isRunning, true, false))
		{
			throw std::runtime_error("RakNet transport is not started");
		}
		std::vector<uint64> guids;
		for (auto connection : _connections)
		{
			guids.push_back(connection.first);
		}
		for (auto pId : guids)
		{
			onDisconnection(pId, "CLIENT_TRANSPORT_STOPPED");
		}

		auto peer = _peer;
		_peer.reset();
		if (peer)
		{
			if (peer->IsActive())
			{
				peer->Shutdown(1000);
			}

			if (_socketDescriptor)
			{
				_socketDescriptor.reset();
			}

			if (_handler)
			{
				_handler.reset();
			}

			_logger->log(LogLevel::Trace, "RakNetTransport", "RakNet transport stopped");
		}
	}

	std::vector<std::string> RakNetTransport::externalAddresses() const
	{
		throw std::runtime_error("not implemented");
		/*std::vector<std::string> addrs;
		addrs.push_back(_peer->GetSystemAddressFromGuid(_peer->GetMyGUID()).ToString(true, ':'));

		if (isRunning())
		{
			addrs.push_back(_peer->GetExternalID(_peer->GetSystemAddressFromGuid(_serverRakNetGUID)).ToString(true, ':'));
		}
		return addrs;*/
	}



	pplx::task<std::shared_ptr<IConnection>> RakNetTransport::connect(std::string endpoint, std::string id, std::string parentId, pplx::cancellation_token ct)
	{
		std::lock_guard<std::mutex> lock(_pendingConnection_mtx);

		auto shouldStart = _pendingConnections.empty();
		ConnectionRequest rq;
		rq.endpoint = endpoint;
		rq.cancellationToken = ct;
		rq.id = id;
		rq.parentId = parentId;

		_pendingConnections.push(rq);
		if (shouldStart)
		{
			startNextPendingConnections();
		}

		return pplx::task<std::shared_ptr<IConnection>>(rq.tce, ct);
	}

	void RakNetTransport::startNextPendingConnections()
	{
		if (_pendingConnections.empty())
		{
			return;
		}

		auto rq = _pendingConnections.front();

		if (rq.cancellationToken.is_canceled())
		{
			_pendingConnections.pop();
			startNextPendingConnections();
		}

		auto endpoint = rq.endpoint;
		auto split = stringSplit(endpoint, ':');
		auto tce = rq.tce;
		if (split.size() < 2)
		{
			tce.set_exception(std::invalid_argument(("Bad server endpoint, no port (" + endpoint + ')').c_str()));
			return;
		}

		std::string portString = split[1];
		if (endpoint.size() - 1 <= portString.size())
		{
			tce.set_exception(std::invalid_argument(("Bad server endpoint, no host (" + endpoint + ')').c_str()));
			return;
		}

		_port = (uint16)std::atoi(portString.c_str());
		if (_port == 0)
		{
			tce.set_exception(std::runtime_error(("Server endpoint port should not be 0 (" + endpoint + ')').c_str()));
			return;
		}

		auto hostStr = split[0];

		if (_peer == nullptr || !_peer->IsActive())
		{
			tce.set_exception(std::runtime_error("Transport not started. Make sure you started it."));
			return;
		}

		auto result = _peer->Connect(hostStr.c_str(), _port, nullptr, 0, nullptr, 0, 12, 500, 30000);
		if (result != RakNet::ConnectionAttemptResult::CONNECTION_ATTEMPT_STARTED)
		{
			tce.set_exception(std::runtime_error((std::string("Bad RakNet connection attempt result (") + std::to_string(result) + ')').c_str()));
			return;
		}
	}

	std::shared_ptr<RakNetConnection> RakNetTransport::onConnection(RakNet::SystemAddress systemAddress, RakNet::RakNetGUID guid, uint64 peerId, const ConnectionRequest& request)
	{
		auto msg = std::string() + systemAddress.ToString(true, ':') + " connected";
		_logger->log(LogLevel::Trace, "RakNetTransport", msg.c_str());

		auto connection = createNewConnection(guid, peerId, request.id);

		_handler->newConnection(connection);

		auto tce = request.tce;
		pplx::task<void>([connection, tce] {
			// Start this asynchronously because we locked the mutex in run and the user can do something that tries to lock again this mutex
			connection->setConnectionState(ConnectionState::Connecting); // we should set connecting state sooner (when the user call connect)
			connection->setConnectionState(ConnectionState::Connected);
			tce.set(connection);
		});

		return connection;
	}

	std::shared_ptr<RakNetConnection> RakNetTransport::onConnection(RakNet::SystemAddress systemAddress, RakNet::RakNetGUID guid, uint64 peerId, std::string connectionId)
	{
		auto msg = std::string() + systemAddress.ToString(true, ':') + " connected";
		_logger->log(LogLevel::Trace, "RakNetTransport", msg.c_str());

		auto connection = createNewConnection(guid, peerId, connectionId);

		_handler->newConnection(connection);

		pplx::task<void>([connection] {
			// Start this asynchronously because we locked the mutex in run and the user can do something that tries to lock again this mutex
			connection->setConnectionState(ConnectionState::Connecting); // we should set connecting state sooner (when the user call connect)
			connection->setConnectionState(ConnectionState::Connected);
		});

		return connection;
	}

	void RakNetTransport::onDisconnection(uint64 guid, std::string reason)
	{
		auto connection = removeConnection(guid);

		if (connection)
		{
			if (!connection->_closeReason.empty())
			{
				reason = connection->_closeReason;
			}

			_handler->closeConnection(connection, reason);

			connection->onClose(reason);


			pplx::task<void>([connection, reason]()
			{
				// Start this asynchronously because we locked the mutex in run and the user can do something that tries to lock again this mutex
				connection->setConnectionState(ConnectionState(ConnectionState::Disconnected, reason));
			});
		}
		else
		{
			auto msg = std::string("Attempting to disconnect from unknown peer (guid: " + std::to_string(guid) + ")");
			_logger->log(LogLevel::Warn, "RakNetTransport", msg);
		}
	}

	void RakNetTransport::onMessageReceived(RakNet::Packet* rakNetPacket)
	{
#if defined(STORMANCER_LOG_PACKETS) && !defined(STORMANCER_LOG_RAKNET_PACKETS)
		std::vector<byte> tempBytes(rakNetPacket->data, rakNetPacket->data + rakNetPacket->length);
		auto bytes = stringifyBytesArray(tempBytes, true, true);
		_logger->log(LogLevel::Trace, "RakNetTransport", "Packet received", bytes.c_str());
#endif

		auto connection = getConnection(rakNetPacket->guid.g);
		byte* data = new byte[rakNetPacket->length];
		std::memcpy(data, rakNetPacket->data, rakNetPacket->length);
		auto stream = new ibytestream(data, (std::streamsize)rakNetPacket->length);

		Packet_ptr packet(new Packet<>(connection, stream), [data, stream](Packet<>* packetPtr) {
			delete packetPtr;
			delete stream;
			delete[] data;
		});

		_onPacketReceived(packet);
	}

	std::shared_ptr<RakNetConnection> RakNetTransport::getConnection(uint64 guid)
	{
		auto it = _connections.find(guid);
		if (it != _connections.end())
		{
			return it->second.connection;
		}
		else
		{
			return nullptr;
		}
	}

	std::shared_ptr<RakNetConnection> RakNetTransport::createNewConnection(RakNet::RakNetGUID raknetGuid, uint64 peerId, std::string key)
	{
		if (_peer)
		{
			int64 cid = peerId;
			auto dr = std::make_shared<DependencyResolver>(_dependencyResolver);
			dr->registerDependency<std::vector<std::weak_ptr<Scene_Impl>>>([](auto dr) {
				return std::make_shared<std::vector<std::weak_ptr<Scene_Impl>>>();
			}, true);
			auto connection = std::shared_ptr<RakNetConnection>(new RakNetConnection(raknetGuid, cid, key, _peer, _logger, dr), deleter<RakNetConnection>());
			RakNet::RakNetGUID guid(connection->guid());
			auto logger = _logger;
			std::weak_ptr<RakNet::RakPeerInterface> weakPeer = _peer;
			ConnectionContainer container;
			container.connection = connection;
			container.onCloseSubscription = connection->onClose.subscribe([logger, weakPeer, cid, guid](std::string reason) {
				//#if defined(_WIN32) && !defined(_XBOX_ONE)
				//				StackWalker sw;
				//				sw.outputFunction = [logger](std::string stack) {
				//					logger->log(LogLevel::Trace, "RaknetTransport", "Triggered Onclose event", stack);
				//				};
				//				sw.ShowCallstack();
				//#endif
				if (auto peer = weakPeer.lock())
				{
					logger->log(LogLevel::Trace, "RakNetTransport", "RakNet connection onClose: " + reason, std::to_string(cid));
					peer->CloseConnection(guid, true);
				}
			});
			_connections[raknetGuid.g] = container;
			return connection;
		}
		return nullptr;
	}

	std::shared_ptr<RakNetConnection> RakNetTransport::removeConnection(uint64 guid)
	{
		auto it = _connections.find(guid);
		if (it != _connections.end())
		{
			auto result = it->second;
			_connections.erase(it);
			return result.connection;
		}
		else
		{
			return nullptr;
		}
	}

	bool RakNetTransport::isRunning() const
	{
		return _isRunning;
	}

	std::string RakNetTransport::name() const
	{
		return _name;
	}



	std::weak_ptr<DependencyResolver> RakNetTransport::dependencyResolver() const
	{
		return _dependencyResolver;
	}

	void RakNetTransport::onPacketReceived(std::function<void(Packet_ptr)> callback)
	{
		_onPacketReceived += callback;
	}

	std::string RakNetTransport::host() const
	{
		return _host.c_str();
	}

	uint16 RakNetTransport::port() const
	{
		auto boundAddress = _peer->GetMyBoundAddress(0);
		return boundAddress.GetPort();
	}

	pplx::task<int> RakNetTransport::sendPing(const std::string& address, pplx::cancellation_token ct)
	{
		return this->sendPing(address, 2, ct);
	}

	bool RakNetTransport::sendPingImpl(const std::string& address)
	{
		auto els = stringSplit(address, ':');
		assert(els.size() >= 2);

		auto port = (uint16)std::atoi(els[1].c_str());

		auto peer = _peer;
		if (peer)
		{
			return peer->Ping(els[0].c_str(), port, false);
		}
		else
		{
			// ping fails: no peer to perform the ping!
			return false;
		}

		/*auto p2 = RakNet::RakPeerInterface::GetInstance();
		p2->Startup(10, new RakNet::SocketDescriptor(), 1);
		auto result = p2->Connect(els[0].c_str(), (uint16)std::atoi(els[1].c_str()),nullptr,0);*/

		//_peer->Connect("127.0.0.1", port, nullptr, 0);
	}

	pplx::task<int> RakNetTransport::sendPing(const std::string& address, const int nb, pplx::cancellation_token ct)
	{
		pplx::task_completion_event<int> tce;

		{
			std::lock_guard<std::mutex> lg(_pendingPingsMutex);
			_pendingPings.emplace(address, tce);
		}

		pplx::task<int> eventSetTask(tce, ct);
		pplx::cancellation_token_source cts = ct.is_cancelable() ? pplx::cancellation_token_source::create_linked_source(ct) : pplx::cancellation_token_source();
		std::vector<pplx::task<bool>> tasks;
		for (int i = 0; i < nb; i++)
		{
			auto wTransport = STRM_WEAK_FROM_THIS();
			auto pingImplTask = taskDelay(std::chrono::milliseconds(300 * i), cts.get_token())
				.then([wTransport, address]()
			{
				auto transport = LockOrThrow(wTransport);
				return transport->sendPingImpl(address);
			}, cts.get_token());
			tasks.push_back(pingImplTask);

			pingImplTask.then([](pplx::task<bool> t)
			{
				try
				{
					t.get();
				}
				catch (...) {}
			});
		}

		auto logger = _logger;
		pplx::when_all(tasks.begin(), tasks.end())
			.then([logger, tce, address](std::vector<bool> results)
		{
			bool sent = false;

			for (bool result : results)
			{
				if (result)
				{
					sent = true;
					break;
				}
			}

			if (!sent)
			{
				logger->log(LogLevel::Debug, "RakNetTransport", "Pings to " + address + " failed: unreachable address.");
				tce.set(-1);
			}
		}, cts.get_token())
			.then([logger, address](pplx::task<void> t) {
			try
			{
				t.get();
			}
			catch (const std::exception&)
			{
				logger->log(LogLevel::Debug, "RakNetTransport", "Pings to " + address + " failed: ping cancelled.");
			}
		});

		auto wTransport = STRM_WEAK_FROM_THIS();
		return cancel_after_timeout(eventSetTask, cts, 1500)
			.then([wTransport, address](pplx::task<int> t)
		{
			auto transport = LockOrThrow(wTransport);
			std::lock_guard<std::mutex> lg(transport->_pendingPingsMutex);
			transport->_pendingPings.erase(address); // destroys the tce and cancels the task
			try
			{
				return t.get();
			}
			catch (std::exception& ex)
			{
				transport->_logger->log(LogLevel::Debug, "RakNetTransport", "Ping to " + address + " failed", ex.what());
				return -1;
			}
		});
	}

	void RakNetTransport::openNat(const std::string& address)
	{
		auto els = stringSplit(address, ':');
		this->_peer->SendTTL(els[0].c_str(), (uint16)std::stoi(els[1]), 3);
	}

	std::vector<std::string> RakNetTransport::getAvailableEndpoints() const
	{
		auto nbIps = this->_peer->GetNumberOfAddresses();
		std::vector<std::string> ips;
		for (unsigned int i = 0; i < nbIps; i++)
		{
			auto boundAddress = _peer->GetMyBoundAddress(0);
			auto ip = std::string(_peer->GetLocalIP(i));
			if (is_ipv4_address(ip))
			{
				ips.push_back(ip + ":" + std::to_string(boundAddress.GetPort()));
			}
		}

		return ips;
	}
};

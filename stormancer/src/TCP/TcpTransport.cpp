#include "stormancer/stdafx.h"
#include "stormancer/TCP/TcpTransport.h"
#include "stormancer/MessageIDTypes.h"

namespace Stormancer
{
	TcpTransport::TcpTransport(DependencyResolver* dependencyResolver)
		: _dependencyResolver(dependencyResolver)
		, _logger(dependencyResolver->resolve<ILogger>())
		, _scheduler(dependencyResolver->resolve<IScheduler>())
		, _name("tcp")
	{
	}

	TcpTransport::~TcpTransport()
	{
		_logger->log(LogLevel::Trace, "TcpTransport", "Deleting TCP transport...");

		if (_isRunning)
		{
			stop();
		}

		if (_listeningThread)
		{
			_listeningThread->detach();
		}

		_logger->log(LogLevel::Trace, "TcpTransport", "TCP transport deleted");
	}

	void TcpTransport::start(std::string type, std::shared_ptr<IConnectionManager> handler, pplx::cancellation_token ct, uint16 port, uint16)
	{
		_logger->log(LogLevel::Trace, "TcpTransport", "Starting TCP transport...");

		if (!compareExchange(_mutex, _isRunning, false, true))
		{
			throw std::runtime_error("TCP transport is already started.");
		}

		if (port > 0)
		{
			throw std::invalid_argument("TCP transport does not support opening a server port.");
		}

		_type = type;
		_handler = handler;
		initialize();

		_scheduler->schedulePeriodic(15, [=]() {
			// action queue
			{
				std::lock_guard<std::mutex> lock(_actionQueueMutex);
				while (!_actionQueue.empty())
				{
					_actionQueue.front()();
					_actionQueue.pop();
				}
			}
			// packet queue
			{
				std::lock_guard<std::mutex> lock(_mutex);
				if (_isRunning)
				{
					run();
				}
			}
		}, ct);

		ct.register_callback([=]()
		{
			stop();
		});

		_logger->log(LogLevel::Trace, "TcpTransport", "TCP transport started");
	}

	pplx::task<std::shared_ptr<IConnection>> TcpTransport::connect(std::string endpoint, pplx::cancellation_token ct)
	{
		std::lock_guard<std::mutex> lock(_mutex);

		std::smatch m;
		std::regex ipRegex("^(([0-9A-F]{1,4}:){7}[0-9A-F]{1,4}|(\\d{1,3}\\.){3}\\d{1,3}):(\\d{1,5})$", std::regex_constants::ECMAScript | std::regex_constants::icase);
		bool res = std::regex_search(endpoint, m, ipRegex);
		if (!res || m.length() < 5)
		{
			throw std::invalid_argument("Bad scene endpoint (" + endpoint + ')');
		}

		//_host = m.str(1);
		_port = (uint16)std::atoi(m.str(4).c_str());
		if (_port == 0)
		{
			throw std::runtime_error("Scene endpoint port should not be 0 (" + endpoint + ')');
		}

		if (_socketId == nullptr)
		{
			throw std::runtime_error("TCP transport not started. Make sure you started it.");
		}

		if (_listeningThread)
		{
			throw std::runtime_error("Connect has already been called on this TCP transport, and TCP transports only supports connection to a single endpoint.");
		}

		auto tce = pplx::task_completion_event<std::weak_ptr<IConnection>>();
		_pendingConnections[endpoint] = tce;

		_listeningThread = std::make_shared<std::thread>(&TcpTransport::listen, this, endpoint);

		return pplx::create_task(tce).then([] (std::weak_ptr<IConnection> connection)
		{
			return connection.lock();
		}, ct);
	}

	bool TcpTransport::isRunning() const
	{
		return _isRunning;
	}

	std::string TcpTransport::name() const
	{
		return _name;
	}

	uint64 TcpTransport::id() const
	{
		return _id;
	}

	DependencyResolver* TcpTransport::dependencyResolver() const
	{
		return _dependencyResolver;
	}

	void TcpTransport::onPacketReceived(std::function<void(Packet_ptr)> callback)
	{
		_onPacketReceived += callback;
	}

	std::string TcpTransport::host() const
	{
		return _host;
	}

	uint16 TcpTransport::port() const
	{
		return _port;
	}

	std::vector<std::string> TcpTransport::externalAddresses() const
	{
		return std::vector<std::string>(1, _host + ":" + std::to_string(_port));
	}

	void TcpTransport::initialize()
	{
		_logger->log(LogLevel::Trace, "TcpTransport", "Initializing TCP transport", _type.c_str());

		auto s = socket__(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (s < 0)
		{
			throw std::runtime_error(std::string("Could not create the tcp socket. Error: " + std::to_string(s)));
		}

		_socketId = std::shared_ptr<SOCKET>(new SOCKET(s), [](SOCKET* socketId)
		{
			closesocket__(*socketId);
			delete(socketId);
		});
	}

	void TcpTransport::run()
	{
		Packet_ptr packet;

		bool shouldLoop = true;
		while (shouldLoop)
		{
			{
				std::lock_guard<std::mutex> lg(_packetQueueMutex);
				shouldLoop = _pendingPackets.size() > 0;

				if (shouldLoop)
				{
					packet = _pendingPackets.front();
					_pendingPackets.pop();
				}
				else
				{
					packet = nullptr;
				}
			}

			if (packet)
			{
				_onPacketReceived(packet);
			}
		}
	}

	void TcpTransport::onConnectionIdReceived(int64 id)
	{
		_id = id;
	}

	void TcpTransport::listen(std::string endpoint)
	{
		_logger->log(LogLevel::Trace, "TcpTransport", "Start listening");

		sockaddr_in sin;



		sin.sin_family = AF_INET;
		if (inet_pton(AF_INET, _host.c_str(), &sin.sin_addr) <= 0)
		{
			_pendingConnections[endpoint].set_exception(std::runtime_error(std::string("Unable to resolve ip ") + endpoint));
			return;
		}
		sin.sin_port = htons(_port);

		auto ret = ::connect__(*_socketId, (sockaddr*)&sin, sizeof(sin));
		if (ret < 0)
		{
			_pendingConnections[endpoint].set_exception(std::runtime_error(std::string("Unable to connect to endpoint ") + endpoint));
			return;
		}

		_connection = createNewConnection(endpoint);
		_pendingConnections[endpoint].set(_connection);

		int readbytes = 0;
		while (isRunning() && _socketId)
		{
			SOCKET socketId = *_socketId;
			int msgLength;
			readbytes = recv__(socketId, (char*)&msgLength, sizeof(msgLength), MSG_WAITALL);
			if (readbytes < sizeof(msgLength))
			{
				std::lock_guard<std::mutex> lg(_actionQueueMutex);
				_actionQueue.push([=]()
				{
					onDisconnection(endpoint, readbytes == 0 ? "remote server disconnected." : "an error occurred");
				});
				break;
			}
			if (msgLength == 0)
			{
				continue;
			}
			auto buffer = new byte[msgLength];

			readbytes = recv__(socketId, (char*)buffer, msgLength, MSG_WAITALL);
			if (readbytes < msgLength)
			{
				std::lock_guard<std::mutex> lg(_actionQueueMutex);
				_actionQueue.push([=]()
				{
					onDisconnection(endpoint, readbytes == 0 ? "remote server disconnected." : "an error occurred");
				});
				delete[] buffer;
				break;
			}

			if (buffer[0] == (byte)MessageIDTypes::ID_CONNECTION_RESULT)
			{
				int64 sid = *(int64*)(buffer + 1);
				_logger->log(LogLevel::Trace, "TcpTransport", "Connection ID received.", std::to_string(sid).c_str());
				onConnectionIdReceived(sid);
				delete[] buffer;
				continue;
			}

			auto stream = new ibytestream(buffer, msgLength);
			Packet_ptr packet(new Packet<>(_connection, stream), [stream, buffer](Packet<>* packetPtr) {
				delete packetPtr;
				delete stream;
				delete[] buffer;
			});

			{
				std::lock_guard<std::mutex> lg(_packetQueueMutex);
				_pendingPackets.push(packet);
			}
		}

		_logger->log(LogLevel::Trace, "TcpTransport", "Stop listening");
	}

	std::shared_ptr<TcpConnection> TcpTransport::createNewConnection(std::string endpoint)
	{
		_logger->log(LogLevel::Trace, "TcpTransport", "Connected", endpoint.c_str());
		uint64 cid = 0;
		std::shared_ptr<TcpConnection> connection = std::make_shared<TcpConnection>(_socketId, cid, _host);
		connection->onClose([=](std::string reason) {
			_logger->log(LogLevel::Trace, "tcpTransport", "Closed ", endpoint.c_str());
		});
		_connection = connection;

		_handler->newConnection(connection);

		connection->setConnectionState(ConnectionState::Connected);

		return connection;
	}

	void TcpTransport::onDisconnection(std::string endpoint, std::string reason)
	{
		auto msg = endpoint + " disconnected.";
		_logger->log(LogLevel::Trace, "TcpTransport", msg.c_str(), reason.c_str());
		if (_connection)
		{
			_handler->closeConnection(_connection, reason);
			_connection->_closeAction(reason);
			_connection->setConnectionState(ConnectionState::Disconnected);
			_connection = nullptr;
		}
	}

	void TcpTransport::stop()
	{
		_logger->log(LogLevel::Trace, "TcpTransport", "Stopping TCP transport...");

		if (!compareExchange(_mutex, _isRunning, true, false))
		{
			throw std::runtime_error("TCP transport is not started");
		}

		_socketId = nullptr;

		if (_listeningThread)
		{
			_listeningThread->join();
			_listeningThread.reset();
		}

		_connection = nullptr;

		if (_handler)
		{
			_handler.reset();
		}

		_logger->log(LogLevel::Trace, "TcpTransport", "TCP transport stopped");
	}

	pplx::task<int> TcpTransport::sendPing(const std::string& /*address*/, pplx::cancellation_token /*ct*/)
	{
		throw std::runtime_error("not implemented");
	}

	void TcpTransport::openNat(const std::string& /*address*/)
	{
		throw std::runtime_error("not implemented");
	}

	std::vector<std::string> TcpTransport::getAvailableEndpoints() const
	{
		throw std::runtime_error("not implemented");
	}
}

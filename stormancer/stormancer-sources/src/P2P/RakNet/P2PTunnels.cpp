#include "stormancer/stdafx.h"
#include "stormancer/P2P/RakNet/P2PTunnelClient.h" // Includes RakNet ; must be included before any mention of windows.h (needed for tasks)
#include "stormancer/P2P/RakNet/P2PTunnels.h"
#include "stormancer/IConnectionManager.h"
#include "stormancer/SystemRequestIDTypes.h"
#include "stormancer/MessageIDTypes.h"
#include "stormancer/SceneImpl.h"
#include "stormancer/Utilities/PointerUtilities.h"
#include "stormancer/P2P/OpenTunnelResult.h"
#include "stormancer/ChannelUidStore.h"
#include "stormancer/Utilities/StringUtilities.h"

namespace Stormancer
{
	P2PTunnels::P2PTunnels(std::shared_ptr<RequestProcessor> sysCall, std::shared_ptr<IConnectionManager> connections,
		std::shared_ptr<Serializer> serializer,
		std::shared_ptr<Configuration> configuration,
		std::shared_ptr<ILogger> logger)
	{
		_logger = logger;
		_sysClient = sysCall;
		_connections = connections;
		_serializer = serializer;
		_config = configuration;
	}

	std::shared_ptr<P2PTunnel> P2PTunnels::createServer(std::string serverId, std::shared_ptr<P2PTunnels> tunnels)
	{
		std::lock_guard<std::mutex> lg(_syncRoot);

		auto tunnel = std::make_shared<P2PTunnel>([tunnels, serverId]() {
			tunnels->destroyServer(serverId);
		});
		tunnel->id = serverId;
		tunnel->ip = "127.0.0.1";
		tunnel->port = _config->serverGamePort;
		tunnel->side = P2PTunnelSide::Host;
		ServerDescriptor descriptor;
		descriptor.id = tunnel->id;
		descriptor.hostName = tunnel->ip;
		descriptor.port = tunnel->port;
		_servers[descriptor.id] = descriptor;

		return tunnel;
	}

	pplx::task<std::shared_ptr<P2PTunnel>> P2PTunnels::openTunnel(std::string sessionId, std::string serverId, pplx::cancellation_token ct)
	{
		auto connection = _connections->getConnection(sessionId);

		if (!connection)
		{
			throw std::runtime_error("No p2p connection established to the target peer");
		}

		auto wP2pTunnels = STORM_WEAK_FROM_THIS();

		return _sysClient->sendSystemRequest<OpenTunnelResult>(connection.get(), (byte)SystemRequestIDTypes::ID_P2P_OPEN_TUNNEL, serverId, ct)
			.then([wP2pTunnels, sessionId, serverId](OpenTunnelResult result)
		{
			auto p2pTunnels = LockOrThrow(wP2pTunnels);

			if (result.useTunnel)
			{
				auto client = std::make_shared<P2PTunnelClient>([wP2pTunnels](P2PTunnelClient* client, RakNet::RNS2RecvStruct* msg)
				{
					auto p2pTunnels = LockOrThrow(wP2pTunnels);
					p2pTunnels->onMsgReceived(client, msg);
				},
					p2pTunnels->_sysClient,
					p2pTunnels->_logger,
					p2pTunnels->_config->tunnelPort,
					p2pTunnels->_config->useIpv6Tunnel);

				client->handle = result.handle;
				client->peerSessionId = sessionId;
				client->serverId = serverId;
				client->serverSide = false;

				{
					std::lock_guard<std::mutex> lg(p2pTunnels->_syncRoot);
					p2pTunnels->_tunnels[std::make_tuple(sessionId, result.handle)] = client;
				}

				byte tunnelHandle = result.handle;
				auto tunnel = std::make_shared<P2PTunnel>([wP2pTunnels, sessionId, tunnelHandle]()
				{
					if (auto p2pTunnels = wP2pTunnels.lock())
					{
						p2pTunnels->destroyTunnel(sessionId, tunnelHandle);
					}
				});
				tunnel->id = serverId;
				tunnel->ip = "127.0.0.1";
				tunnel->port = (uint16)client->socket->GetBoundAddress().GetPort();
				return tunnel;
			}
			else
			{
				auto tunnel = std::make_shared<P2PTunnel>([result]() {});
				auto el = stringSplit(result.endpoint, ':');
				tunnel->id = serverId;
				tunnel->ip = el[0];
				tunnel->port = (uint16)std::stoi(el[1]);
				return tunnel;
			}
		}, ct);
	}

	byte P2PTunnels::addClient(std::string serverId, std::string clientPeerSessionId)
	{
		std::lock_guard<std::mutex> lg(_syncRoot);

		auto serverIt = _servers.find(serverId);
		if (serverIt == _servers.end())
		{
			throw std::runtime_error("The server does not exist");
		}

		for (byte handle = 0; handle < 255; handle++)
		{
			peerHandle key(clientPeerSessionId, handle);
			if (_tunnels.find(key) == _tunnels.end())
			{
				auto client = std::make_shared<P2PTunnelClient>(
					[=](P2PTunnelClient* client, RakNet::RNS2RecvStruct* msg) { this->onMsgReceived(client, msg); },
					_sysClient,
					_logger,
					(uint16)0,
					_config->useIpv6Tunnel);

				client->handle = handle;
				client->peerSessionId = clientPeerSessionId;
				client->serverId = serverId;
				client->serverSide = true;
				client->hostPort = serverIt->second.port;
				_tunnels[key] = client;
				return handle;
			}
		}

		throw std::runtime_error("Unable to create tunnel handle : Too many tunnels opened between the peers.");
	}

	void P2PTunnels::closeTunnel(byte handle, std::string peerId)
	{
		std::lock_guard<std::mutex> lg(_syncRoot);
		_tunnels.erase(std::make_tuple(peerId, handle));
	}

	pplx::task<void> P2PTunnels::destroyServer(std::string serverId)
	{
		std::vector<pplx::task<void>> tasks;
		std::vector<peerHandle> itemsToDelete;

		{
			std::lock_guard<std::mutex> lg(_syncRoot);

			for (auto tunnel : _tunnels)
			{
				if (tunnel.second->serverId == serverId && tunnel.second->serverSide)
				{
					auto connection = _connections->getConnection(tunnel.second->peerSessionId);
					itemsToDelete.push_back(tunnel.first);
					if (connection)
					{
						auto handle = std::get<1>(tunnel.first);
						tasks.push_back(_sysClient->sendSystemRequest(connection.get(), (byte)SystemRequestIDTypes::ID_P2P_CLOSE_TUNNEL, handle).then([](pplx::task<void> t) {
							try
							{
								t.get();
							}
							catch (...) {}
						}));
					}

				}
			}

			for (auto item : itemsToDelete)
			{
				_tunnels.erase(item);
			}
		}

		return pplx::when_all(tasks.begin(), tasks.end());
	}

	pplx::task<void> P2PTunnels::destroyTunnel(std::string peerId, byte handle)
	{
		std::lock_guard<std::mutex> lg(_syncRoot);

		auto it = _tunnels.find(std::make_tuple(peerId, handle));
		if (it != _tunnels.end())
		{
			auto client = (*it).second;
			auto connection = _connections->getConnection(client->peerSessionId);
			_tunnels.erase(it);
			if (connection)
			{
				return _sysClient->sendSystemRequest(connection.get(), (byte)SystemRequestIDTypes::ID_P2P_CLOSE_TUNNEL, handle)
					.then([](pplx::task<void> t) {
					try
					{
						t.wait();
					}
					catch (...)
					{
					}
				});
			}
		}

		return pplx::task_from_result();
	}

	void P2PTunnels::receiveFrom(std::string id, ibytestream& stream)
	{
		std::lock_guard<std::mutex> lg(_syncRoot);

		byte handle;
		stream.read(&handle, 1);
		byte buffer[1464];
		auto read = stream.readsome(buffer, 1464);

		auto itTunnel = _tunnels.find(std::make_tuple(id, handle));
		if (itTunnel != _tunnels.end())
		{
			auto client = (*itTunnel).second;
			if (client)
			{
				auto socket = client->socket;
				if (socket)
				{
					RakNet::RNS2_SendParameters bsp;
					bsp.data = reinterpret_cast<char*>(buffer);
					bsp.length = (int)read;
					bsp.systemAddress.FromStringExplicitPort("127.0.0.1", client->hostPort, socket->GetBoundAddress().GetIPVersion());
					std::string peerPort = std::to_string(client->hostPort);
					//_logger->log(LogLevel::Trace, "p2p.tunnel","Sending data to game client");
					(*itTunnel).second->socket->Send(&bsp, _FILE_AND_LINE_);
				}
			}
			else
			{
				_tunnels.erase(itTunnel);
			}
		}
	}

	void P2PTunnels::onMsgReceived(P2PTunnelClient* client, RakNet::RNS2RecvStruct* recvStruct)
	{
		auto connection = _connections->getConnection(client->peerSessionId);
		if (connection)
		{

			if (client->hostPort != recvStruct->systemAddress.GetPort())
			{
				client->hostPort = recvStruct->systemAddress.GetPort();
			}
			std::stringstream ss;
			ss << "P2PTunnels_" << connection->sessionId();
			int channelUid = connection->dependencyResolver().resolve<ChannelUidStore>()->getChannelUid(ss.str());
			//_logger->log(LogLevel::Trace, "p2p.tunnel", "Sending data to tunnel");
			connection->send([=](obytestream& stream)
			{
				stream << (byte)MessageIDTypes::ID_P2P_TUNNEL;
				stream << client->handle;
				stream.write(reinterpret_cast<const byte*>(recvStruct->data), static_cast<std::streamsize>(recvStruct->bytesRead));
			}, channelUid, PacketPriority::IMMEDIATE_PRIORITY, PacketReliability::UNRELIABLE);
		}
	}

	std::size_t PeerHandle_hash::operator()(const peerHandle & k) const
	{
		auto h = std::hash<std::string>()(std::get<0>(k));
		return (size_t)(h ^ std::get<1>(k));
	}

	bool PeerHandle_equal::operator()(const peerHandle & v0, const peerHandle & v1) const
	{
		return v0 == v1;
	}
}

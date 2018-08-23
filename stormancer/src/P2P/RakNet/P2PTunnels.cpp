#include "stormancer/stdafx.h"
#include "stormancer/P2P/RakNet/P2PTunnels.h"
#include "stormancer/IConnectionManager.h"
#include "stormancer/SystemRequestIDTypes.h"
#include "stormancer/MessageIDTypes.h"
#include "stormancer/P2P/RakNet/P2PTunnelClient.h"
#include "stormancer/Scene.h"
#include "stormancer/SafeCapture.h"
#include "stormancer/P2P/OpenTunnelResult.h"

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

	pplx::task<std::shared_ptr<P2PTunnel>> P2PTunnels::openTunnel(uint64 connectionId, std::string serverId, pplx::cancellation_token ct)
	{
		auto connection = _connections->getConnection(connectionId);

		if (!connection)
		{
			throw std::runtime_error("No p2p connection established to the target peer");
		}

		auto wP2pTunnels = STRM_WEAK_FROM_THIS();

		return _sysClient->sendSystemRequest<OpenTunnelResult>(connection.get(), (byte)SystemRequestIDTypes::ID_P2P_OPEN_TUNNEL, serverId, ct)
			.then([wP2pTunnels, connectionId, serverId](OpenTunnelResult result)
		{
			auto p2pTunnels = LockOrThrow(wP2pTunnels);

			if (result.useTunnel)
			{
				auto client = std::make_shared<P2PTunnelClient>([wP2pTunnels](P2PTunnelClient* client, RakNet::RNS2RecvStruct* msg)
				{
					auto p2pTunnels = LockOrThrow(wP2pTunnels);
					p2pTunnels->onMsgReceived(client, msg);
				}, p2pTunnels->_sysClient, p2pTunnels->_logger);
				client->handle = result.handle;
				client->peerId = connectionId;
				client->serverId = serverId;
				client->serverSide = false;

				{
					std::lock_guard<std::mutex> lg(p2pTunnels->_syncRoot);
					p2pTunnels->_tunnels[std::make_tuple(connectionId, result.handle)] = client;
				}

				auto tunnel = std::make_shared<P2PTunnel>([wP2pTunnels, connectionId, result]()
				{
					auto p2pTunnels = LockOrThrow(wP2pTunnels);
					p2pTunnels->destroyTunnel(connectionId, result.handle);
				});
				tunnel->id = serverId;
				tunnel->ip = "127.0.0.1";
				tunnel->port = (uint16)client->socket->GetBoundAddress().GetPort();
				return tunnel;
			}
			else
			{
				auto tunnel = std::make_shared<P2PTunnel>([connectionId, result]() {});
				auto el = stringSplit(result.endpoint, ':');
				tunnel->id = serverId;
				tunnel->ip = el[0];
				tunnel->port = (uint16)std::stoi(el[1]);
				return tunnel;
			}
		}, ct);
	}

	byte P2PTunnels::addClient(std::string serverId, uint64 clientPeerId)
	{
		std::lock_guard<std::mutex> lg(_syncRoot);

		auto serverIt = _servers.find(serverId);
		if (serverIt == _servers.end())
		{
			throw std::runtime_error("The server does not exist");
		}

		for (byte handle = 0; handle < 255; handle++)
		{
			peerHandle key(clientPeerId, handle);
			if (_tunnels.find(key) == _tunnels.end())
			{
				auto client = std::make_shared<P2PTunnelClient>([=](P2PTunnelClient* client, RakNet::RNS2RecvStruct* msg) { this->onMsgReceived(client, msg); }, _sysClient, _logger);
				client->handle = handle;
				client->peerId = clientPeerId;
				client->serverId = serverId;
				client->serverSide = true;
				client->hostPort = serverIt->second.port;
				_tunnels[key] = client;
				return handle;
			}
		}

		throw std::runtime_error("Unable to create tunnel handle : Too many tunnels opened between the peers.");
	}

	void P2PTunnels::closeTunnel(byte handle, uint64 peerId)
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
					auto connection = _connections->getConnection(tunnel.second->peerId);
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

	pplx::task<void> P2PTunnels::destroyTunnel(uint64 peerId, byte handle)
	{
		std::lock_guard<std::mutex> lg(_syncRoot);

		auto it = _tunnels.find(std::make_tuple(peerId, handle));
		if (it != _tunnels.end())
		{
			auto client = (*it).second;
			auto connection = _connections->getConnection(client->peerId);
			_tunnels.erase(it);
			if (connection)
			{
				return _sysClient->sendSystemRequest(connection.get(), (byte)SystemRequestIDTypes::ID_P2P_CLOSE_TUNNEL, handle)
					.then([](pplx::task<void> t) {
					try
					{
						t.get();
					}
					catch (...)
					{
					}
				});
			}
		}

		return pplx::task_from_result();
	}

	void P2PTunnels::receiveFrom(uint64 id, ibytestream* stream)
	{
		std::lock_guard<std::mutex> lg(_syncRoot);

		byte handle;
		stream->read(&handle, 1);
		byte buffer[1464];
		auto read = stream->readsome(buffer, 1464);

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
					bsp.data = (char*)buffer;
					bsp.length = (int)read;
					bsp.systemAddress.FromStringExplicitPort("127.0.0.1", client->hostPort, socket->GetBoundAddress().GetIPVersion());

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
		auto connection = _connections->getConnection(client->peerId);
		if (connection)
		{
			if (client->hostPort == 0)
			{
				client->hostPort = recvStruct->systemAddress.GetPort();
			}
			std::stringstream ss;
			ss << "P2PTunnels_" << connection->id();
			int channelUid = connection->getChannelUidStore().getChannelUid(ss.str());
			connection->send([=](obytestream* stream) {
				(*stream) << (byte)MessageIDTypes::ID_P2P_TUNNEL;
				(*stream) << client->handle;
				stream->write(recvStruct->data, recvStruct->bytesRead);
			}, channelUid, PacketPriority::IMMEDIATE_PRIORITY, PacketReliability::UNRELIABLE);
		}
	}

	std::size_t PeerHandle_hash::operator()(const peerHandle & k) const
	{
		return (size_t)(std::get<0>(k) ^ std::get<1>(k));
	}

	bool PeerHandle_equal::operator()(const peerHandle & v0, const peerHandle & v1) const
	{
		return v0 == v1;
	}
};

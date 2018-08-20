#include "stormancer/stdafx.h"
#include "stormancer/P2P/P2PRequestModule.h"
#include "stormancer/P2P/ConnectivityCandidate.h"
#include "stormancer/P2P/OpenRelayParameters.h"
#include "stormancer/P2P/RelayConnection.h"
#include "stormancer/SystemRequestIDTypes.h"
#include "stormancer/P2P/RakNet/P2PTunnels.h"
#include "stormancer/P2P/OpenTunnelResult.h"

namespace Stormancer
{
	P2PRequestModule::P2PRequestModule(
		std::shared_ptr<ITransport> transport,
		std::shared_ptr<IConnectionManager> connections,
		std::shared_ptr<P2PSessions> sessions,
		std::shared_ptr<Serializer> serializer,
		std::shared_ptr<P2PTunnels> tunnels,
		std::shared_ptr<ILogger> logger,
		std::shared_ptr<Configuration> config
	)
		: _transport(transport)
		, _connections(connections)
		, _sessions(sessions)
		, _serializer(serializer)
		, _tunnels(tunnels)
		, _logger(logger)
		, _config(config)
	{
	}

	void P2PRequestModule::registerModule(RequestModuleBuilder* builder)
	{
		auto logger = _logger;
		auto config = _config;
		auto transport = _transport;
		auto serializer = _serializer;
		auto sessions = _sessions;
		auto connections = _connections;
		auto tunnels = _tunnels;

		builder->service((byte)SystemRequestIDTypes::ID_P2P_GATHER_IP, [config, transport, serializer](RequestContext* ctx)
		{
			std::vector<std::string> endpoints;
			if (config->dedicatedServerEndpoint != "")
			{
				endpoints.push_back(config->dedicatedServerEndpoint + ":" + std::to_string(config->clientSDKPort));
			}
			else if (config->enableNatPunchthrough == false)
			{
				endpoints.clear();
			}
			else
			{
				endpoints = transport->getAvailableEndpoints();
			}
			ctx->send([serializer, endpoints](obytestream* stream)
			{
				serializer->serialize(stream, endpoints);
			});
			return pplx::task_from_result();
		});

		builder->service((byte)SystemRequestIDTypes::ID_P2P_CREATE_SESSION, [serializer, sessions](RequestContext* ctx)
		{
			auto sessionIdVector = serializer->deserializeOne<std::vector<char>>(ctx->inputStream());

			std::string sessionId(sessionIdVector.begin(), sessionIdVector.end());

			auto peerId = serializer->deserializeOne<uint64>(ctx->inputStream());

			//TODO : use true scene id when it's available from the server
			std::string sceneId = "";
			//auto sceneId = _serializer->deserializeOne<std::string>(ctx->inputStream());			
			P2PSession session;
			session.sessionId = sessionIdVector;
			session.sceneId = sceneId;
			session.remotePeer = peerId;
			sessions->createSession(sessionId, session);
			ctx->send(Writer());
			return pplx::task_from_result();
		});

		builder->service((byte)SystemRequestIDTypes::ID_P2P_CLOSE_SESSION, [serializer, sessions](RequestContext* ctx)
		{
			auto sessionIdVector = serializer->deserializeOne<std::vector<byte>>(ctx->inputStream());

			std::string sessionId(sessionIdVector.begin(), sessionIdVector.end());

			sessions->closeSession(sessionId);
			ctx->send(Writer());

			return pplx::task_from_result();
		});

		builder->service((byte)SystemRequestIDTypes::ID_P2P_TEST_CONNECTIVITY_CLIENT, [logger, serializer, connections, config, transport](RequestContext* ctx)
		{
			auto candidate = serializer->deserializeOne<ConnectivityCandidate>(ctx->inputStream());

			logger->log(LogLevel::Debug, "p2p", "Starting connectivity test (CLIENT) " + candidate.clientEndpointCandidate.address + " => " + candidate.listeningEndpointCandidate.address);

			auto connection = connections->getConnection(candidate.listeningPeer);
			if (connection && connection->getConnectionState() == ConnectionState::Connected)
			{
				ctx->send([serializer](obytestream* s)
				{
					serializer->serialize(s, 0);
				});
				return pplx::task_from_result();
			}
			if (config->enableNatPunchthrough == false)
			{
				if (candidate.listeningEndpointCandidate.address.substr(0, 9) == "127.0.0.1")
				{
					ctx->send([serializer](obytestream* stream)
					{
						serializer->serialize(stream, -1);
					});
					return pplx::task_from_result();
				}
				else
				{
					ctx->send([serializer](obytestream* stream)
					{
						serializer->serialize(stream, 1);
					});
					return pplx::task_from_result();
				}
			}
			else if (config->dedicatedServerEndpoint != "")
			{
				ctx->send([serializer](obytestream* stream)
				{
					serializer->serialize(stream, -1);
				});
				return pplx::task_from_result();
			}
			else
			{
				return transport->sendPing(candidate.listeningEndpointCandidate.address)
					.then([logger, candidate, serializer, ctx](int latency)
				{
					logger->log(LogLevel::Debug, "p2p", "Connectivity test complete : " + candidate.clientEndpointCandidate.address + " => " + candidate.listeningEndpointCandidate.address + " ping : " + std::to_string(latency));
					ctx->send([serializer, latency](obytestream* stream)
					{
						serializer->serialize(stream, latency);
					});
				});
			}
		});

		builder->service((byte)SystemRequestIDTypes::ID_P2P_TEST_CONNECTIVITY_HOST, [serializer, logger, connections, config, transport](RequestContext* ctx)
		{
			auto candidate = serializer->deserializeOne<ConnectivityCandidate>(ctx->inputStream());

			logger->log(LogLevel::Debug, "p2p", "Starting connectivity test (LISTENER) " + candidate.clientEndpointCandidate.address + " => " + candidate.listeningEndpointCandidate.address);

			auto connection = connections->getConnection(candidate.clientPeer);
			if (connection && connection->getConnectionState() == ConnectionState::Connected)
			{
				ctx->send(Writer());
				return pplx::task_from_result();
			}
			if (config->dedicatedServerEndpoint == "" && config->enableNatPunchthrough)
			{
				transport->openNat(candidate.clientEndpointCandidate.address);
			}
			ctx->send(Writer());
			return pplx::task_from_result();
		});

		builder->service((byte)SystemRequestIDTypes::ID_P2P_CONNECT_HOST, [serializer, connections, sessions, logger](RequestContext* ctx)
		{
			auto candidate = serializer->deserializeOne<ConnectivityCandidate>(ctx->inputStream());

			std::string sessionId(candidate.sessionId.begin(), candidate.sessionId.end());

			auto connection = connections->getConnection(candidate.clientPeer);
			if (connection && connection->getConnectionState() == ConnectionState::Connected)
			{
				sessions->updateSessionState(sessionId, P2PSessionState::Connected);
				ctx->send(Writer());
				return pplx::task_from_result();
			}

			logger->log(LogLevel::Debug, "p2p", "Waiting connection " + candidate.clientEndpointCandidate.address + " => " + candidate.listeningEndpointCandidate.address);
			connections->addPendingConnection(candidate.clientPeer)
				.then([sessions, sessionId](std::shared_ptr<IConnection>)
			{
				sessions->updateSessionState(sessionId, P2PSessionState::Connected);
			})
				.then([logger](pplx::task<void> t)
			{
				try
				{
					t.get();
				}
				catch (const std::exception& ex)
				{
					logger->log(LogLevel::Debug, "p2p.connect_host", ex.what());
				}
			});

			ctx->send(Writer());
			return pplx::task_from_result();
		});

		builder->service((byte)SystemRequestIDTypes::ID_P2P_CONNECT_CLIENT, [serializer, logger, connections, sessions, transport](RequestContext* ctx)
		{
			auto candidate = serializer->deserializeOne<ConnectivityCandidate>(ctx->inputStream());

			std::string sessionId(candidate.sessionId.begin(), candidate.sessionId.end());

			logger->log(LogLevel::Debug, "p2p", "Starting P2P client connection client peer =" + std::to_string(candidate.clientPeer));

			auto connection = connections->getConnection(candidate.listeningPeer);
			if (connection && connection->getConnectionState() == ConnectionState::Connected)
			{
				sessions->updateSessionState(sessionId, P2PSessionState::Connected);
				ctx->send([serializer](obytestream* stream)
				{
					serializer->serialize(stream, true);
				});
				return pplx::task_from_result();
			}

			logger->log(LogLevel::Debug, "p2p", "Connecting... " + candidate.clientEndpointCandidate.address + " => " + candidate.listeningEndpointCandidate.address);
			connections->addPendingConnection(candidate.listeningPeer)
				.then([sessions, sessionId](std::shared_ptr<IConnection>)
			{
				sessions->updateSessionState(sessionId, P2PSessionState::Connected);
			})
				.then([logger](pplx::task<void> t)
			{
				try
				{
					t.get();
				}
				catch (const std::exception& ex)
				{
					logger->log(LogLevel::Debug, "p2p.connect_host", ex.what());
				}
			});

			return transport->connect(candidate.listeningEndpointCandidate.address)
				.then([logger, serializer, ctx](pplx::task<std::shared_ptr<IConnection>> t)
			{
				try
				{
					auto connection = t.get();
					logger->log(LogLevel::Trace, "p2p", "Successfully completed connection to " + std::to_string(connection->id()), "");
					ctx->send([serializer, connection](obytestream* stream)
					{
						bool connected = connection && connection->getConnectionState() == ConnectionState::Connected;
						serializer->serialize(stream, connected);
					});
				}
				catch (const std::exception& ex)
				{
					logger->log(LogLevel::Error, "p2p", "Connection attempt failed: ", ex.what());
					throw;
				}
			});
		});

		builder->service((byte)SystemRequestIDTypes::ID_P2P_OPEN_TUNNEL, [serializer, config, tunnels](RequestContext* ctx)
		{
			auto serverId = serializer->deserializeOne<std::string>(ctx->inputStream());

			auto peerId = ctx->packet()->connection->id();
			if (!config->hasPublicIp())
			{
				auto handle = tunnels->addClient(serverId, peerId);

				ctx->send([serializer, handle](obytestream* stream)
				{
					OpenTunnelResult result;
					result.useTunnel = true;
					result.handle = handle;
					serializer->serialize(stream, result);
				});
			}
			else
			{
				std::string endpoint = config->getIp_Port();//Dedicated server IP:port
				ctx->send([serializer, endpoint](obytestream* stream)
				{
					OpenTunnelResult result;
					result.useTunnel = false;
					result.endpoint = endpoint;
					serializer->serialize(stream, result);
				});
			}
			return pplx::task_from_result();
		});

		builder->service((byte)SystemRequestIDTypes::ID_P2P_CLOSE_TUNNEL, [serializer, tunnels](RequestContext* ctx)
		{
			auto handle = serializer->deserializeOne<byte>(ctx->inputStream());

			tunnels->closeTunnel(handle, ctx->packet()->connection->id());
			return pplx::task_from_result();
		});

		builder->service((byte)SystemRequestIDTypes::ID_P2P_RELAY_OPEN, [serializer, connections, sessions](RequestContext* ctx)
		{
			auto relay = serializer->deserializeOne<OpenRelayParameters>(ctx->inputStream());

			connections->newConnection(std::make_shared<RelayConnection>(connections->getConnection(0), relay.remotePeerAddress, relay.remotePeerId));
			std::string sessionId(relay.sessionId.begin(), relay.sessionId.end());
			sessions->updateSessionState(sessionId, P2PSessionState::Connected);
			ctx->send(Writer());
			return pplx::task_from_result();
		});

		builder->service((byte)SystemRequestIDTypes::ID_P2P_RELAY_CLOSE, [serializer, connections](RequestContext* ctx)
		{
			auto peerId = serializer->deserializeOne<uint64>(ctx->inputStream());

			auto connection = connections->getConnection(peerId);
			if (connection)
			{
				connections->closeConnection(connection, "");
			}
			ctx->send([](obytestream*) {});
			return pplx::task_from_result();
		});
	}
};

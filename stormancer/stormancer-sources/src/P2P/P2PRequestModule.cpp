#include "stormancer/stdafx.h"
#include "stormancer/P2P/P2PRequestModule.h"
#include "stormancer/P2P/ConnectivityCandidate.h"
#include "stormancer/P2P/OpenRelayParameters.h"
#include "stormancer/P2P/RelayConnection.h"
#include "stormancer/SystemRequestIDTypes.h"
#include "stormancer/P2P/RakNet/P2PTunnels.h"
#include "stormancer/P2P/OpenTunnelResult.h"
#include "stormancer/Client.h"
#include "stormancer/SafeCapture.h"
#include "stormancer/P2P/P2PConnectToSceneMessage.h"
#include "stormancer/MessageIDTypes.h"
#include <cpprest/asyncrt_utils.h>

namespace Stormancer
{
	P2PRequestModule::P2PRequestModule(
		std::shared_ptr<ITransport> transport,
		std::shared_ptr<IConnectionManager> connections,
		std::shared_ptr<P2PSessions> sessions,
		std::shared_ptr<Serializer> serializer,
		std::shared_ptr<P2PTunnels> tunnels,
		std::shared_ptr<ILogger> logger,
		std::shared_ptr<Configuration> config,
		std::weak_ptr<Client> wClient
	)
		: _transport(transport)
		, _connections(connections)
		, _sessions(sessions)
		, _serializer(serializer)
		, _tunnels(tunnels)
		, _logger(logger)
		, _config(config)
		, _client(wClient)
	{
	}

	void P2PRequestModule::registerModule(RequestModuleBuilder& builder)
	{
		auto logger = _logger;
		auto config = _config;
		auto transport = _transport;
		auto serializer = _serializer;
		auto sessions = _sessions;
		auto connections = _connections;
		std::weak_ptr<P2PTunnels> tunnels = _tunnels;
		auto wClient = _client;

		builder.service((byte)SystemRequestIDTypes::ID_P2P_GATHER_IP, [config, transport, serializer](std::shared_ptr<RequestContext> ctx)
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
			ctx->send([serializer, endpoints](obytestream& stream)
			{
				serializer->serialize(stream, endpoints);
			});
			return pplx::task_from_result();
		});

		builder.service((byte)SystemRequestIDTypes::ID_CONNECT_TO_SCENE, [wClient, serializer](std::shared_ptr<RequestContext> ctx)
		{
			P2PConnectToSceneMessage connectToSceneMessage;
			serializer->deserialize(ctx->inputStream(), connectToSceneMessage);
			auto connection = ctx->packet()->connection;
			if (auto client = wClient.lock())
			{
				return client->getConnectedScene(connectToSceneMessage.sceneId)
					.then([connection, connectToSceneMessage, ctx, serializer](std::shared_ptr<Scene> scene)
				{
					if (!scene)
					{
						return pplx::task_from_exception<void>(std::runtime_error("Scene not found. Cannot connect to peer"));
					}
					auto p2pService = scene->dependencyResolver().resolve<P2PService>();
					auto& handles = *connection->dependencyResolver().resolve<std::vector<std::weak_ptr<Scene_Impl>>>();
					uint8 handle = 0;
					bool success = false;

					for (uint8 i = 0; i < 150; i++)
					{
						if (!handles[i].lock())
						{
							handle = i + (uint8)MessageIDTypes::ID_SCENES;
							handles[i] = std::static_pointer_cast<Scene_Impl>(scene);
							success = true;
							break;
						}
					}
					if (!success)
					{
						return pplx::task_from_exception<void>(std::runtime_error("Failed to generate handle for scene."));
					}

					P2PConnectToSceneMessage connectToSceneResponse;
					connectToSceneResponse.sceneId = scene->address().toUri();
					connectToSceneResponse.sceneHandle = handle;
					for (auto r : scene->localRoutes())
					{
						if ((byte)r->filter() & (byte)MessageOriginFilter::Peer)
						{
							RouteDto routeDto;
							routeDto.Handle = r->handle();
							routeDto.Name = r->name();
							routeDto.Metadata = r->metadata();
							connectToSceneResponse.routes << routeDto;
						}
					}
					connectToSceneResponse.connectionMetadata = ctx->packet()->connection->metadata();
					connectToSceneResponse.sceneMetadata = scene->getSceneMetadata();
					ctx->send([serializer, &connectToSceneResponse](obytestream& stream)
					{
						serializer->serialize(stream, connectToSceneResponse);
					});

					std::static_pointer_cast<Scene_Impl>(scene)->peerConnected(connection, p2pService, connectToSceneMessage);

					return pplx::task_from_result();
				});
			}
			else
			{
				return pplx::task_from_result();
			}
		});

		builder.service((byte)SystemRequestIDTypes::ID_CONNECTED_TO_SCENE, [wClient, serializer](std::shared_ptr<RequestContext> ctx) {
			auto sceneId = serializer->deserializeOne<std::string>(ctx->inputStream());
			if (auto client = wClient.lock())
			{
				ctx->send(StreamWriter{});

				return client->getConnectedScene(sceneId)
					.then([ctx](std::shared_ptr<Scene> scene)
				{
					if (scene)
					{
						std::static_pointer_cast<Scene_Impl>(scene)->raisePeerConnected(ctx->packet()->connection->sessionId());
					}
				});
			}
			else
			{
				return pplx::task_from_result();
			}
		});

		builder.service((byte)SystemRequestIDTypes::ID_DISCONNECT_FROM_SCENE, [serializer, wClient](std::shared_ptr<RequestContext> ctx) {
			std::string sceneId, reason;
			byte sceneHandle;
			serializer->deserialize(ctx->inputStream(), sceneId, reason, sceneHandle);
			if (auto client = wClient.lock())
			{
				return client->disconnect(ctx->packet()->connection, sceneHandle, true, reason);
			}
			else
			{
				return pplx::task_from_result();
			}
		});

		builder.service((byte)SystemRequestIDTypes::ID_P2P_CREATE_SESSION, [serializer, sessions](std::shared_ptr<RequestContext> ctx)
		{
			serializer->deserializeOne<std::vector<byte>>(ctx->inputStream()); // DEPRECATED (backward compatibility)
			serializer->deserializeOne<uint64>(ctx->inputStream()); // DEPRECATED (backward compatibility)
			serializer->deserializeOne<std::string>(ctx->inputStream()); // DEPRECATED (backward compatibility)
			P2PSession p2pSession = serializer->deserializeOne<P2PSession>(ctx->inputStream());
			std::string p2pSessionId = utility::conversions::to_utf8string(utility::conversions::to_base64(p2pSession.SessionId));
			sessions->createSession(p2pSessionId, p2pSession);
			ctx->send(StreamWriter());
			return pplx::task_from_result();
		});

		builder.service((byte)SystemRequestIDTypes::ID_P2P_CLOSE_SESSION, [serializer, sessions](std::shared_ptr<RequestContext> ctx)
		{
			auto p2pSessionIdVector = serializer->deserializeOne<std::vector<byte>>(ctx->inputStream());
			std::string p2pSessionId = utility::conversions::to_utf8string(utility::conversions::to_base64(p2pSessionIdVector));
			sessions->closeSession(p2pSessionId);
			ctx->send(StreamWriter());
			return pplx::task_from_result();
		});

		builder.service((byte)SystemRequestIDTypes::ID_P2P_TEST_CONNECTIVITY_CLIENT, [logger, serializer, connections, config, transport](std::shared_ptr<RequestContext> ctx)
		{
			auto candidate = serializer->deserializeOne<ConnectivityCandidate>(ctx->inputStream());

			logger->log(LogLevel::Debug, "p2p", "Starting connectivity test (CLIENT) ");

			auto connection = connections->getConnection(candidate.listeningPeer);
			if (connection && connection->getConnectionState() == ConnectionState::Connected)
			{
				ctx->send([serializer](obytestream& s)
				{
					serializer->serialize(s, 0);
				});
				return pplx::task_from_result();
			}
			if (config->enableNatPunchthrough == false)
			{
				if (candidate.listeningEndpointCandidate.address.substr(0, 9) == "127.0.0.1")
				{
					ctx->send([serializer](obytestream& stream)
					{
						serializer->serialize(stream, -1);
					});
					return pplx::task_from_result();
				}
				else
				{
					ctx->send([serializer](obytestream& stream)
					{
						serializer->serialize(stream, 1);
					});
					return pplx::task_from_result();
				}
			}
			else if (config->dedicatedServerEndpoint != "")
			{
				ctx->send([serializer](obytestream& stream)
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
					logger->log(LogLevel::Debug, "p2p", "Connectivity test complete  ping : " + std::to_string(latency));
					ctx->send([serializer, latency](obytestream& stream)
					{
						serializer->serialize(stream, latency);
					});
				});
			}
		});

		builder.service((byte)SystemRequestIDTypes::ID_P2P_TEST_CONNECTIVITY_HOST, [serializer, logger, connections, config, transport](std::shared_ptr<RequestContext> ctx)
		{
			auto candidate = serializer->deserializeOne<ConnectivityCandidate>(ctx->inputStream());

			logger->log(LogLevel::Debug, "p2p", "Starting connectivity test (LISTENER) ");

			auto connection = connections->getConnection(candidate.clientPeer);
			if (connection && connection->getConnectionState() == ConnectionState::Connected)
			{
				ctx->send(StreamWriter());
				return pplx::task_from_result();
			}
			if (config->dedicatedServerEndpoint == "" && config->enableNatPunchthrough)
			{
				transport->openNat(candidate.clientEndpointCandidate.address);
			}
			ctx->send(StreamWriter());
			return pplx::task_from_result();
		});

		builder.service((byte)SystemRequestIDTypes::ID_P2P_CONNECT_HOST, [serializer, connections, sessions, logger](std::shared_ptr<RequestContext> ctx)
		{
			auto candidate = serializer->deserializeOne<ConnectivityCandidate>(ctx->inputStream());

			auto sessionId = utility::conversions::to_utf8string(utility::conversions::to_base64(candidate.sessionId));
			auto clientSessionId = utility::conversions::to_utf8string(utility::conversions::to_base64(candidate.clientPeerSessionId));

			auto connection = connections->getConnection(candidate.clientPeer);
			if (connection && connection->getConnectionState() == ConnectionState::Connected)
			{
				sessions->updateSessionState(sessionId, P2PSessionState::Connected);
				ctx->send(StreamWriter());
				return pplx::task_from_result();
			}

			logger->log(LogLevel::Debug, "p2p", "Waiting connection ");
			connections->addPendingConnection(candidate.clientPeer, clientSessionId)
				.then([sessions, sessionId](std::shared_ptr<IConnection> connection)
			{
				connection->setMetadata("type", "p2p");
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

			ctx->send(StreamWriter());
			return pplx::task_from_result();
		});

		builder.service((byte)SystemRequestIDTypes::ID_P2P_CONNECT_CLIENT, [serializer, logger, connections, sessions, transport](std::shared_ptr<RequestContext> ctx)
		{
			auto candidate = serializer->deserializeOne<ConnectivityCandidate>(ctx->inputStream());

			auto p2pSessionId = utility::conversions::to_utf8string(utility::conversions::to_base64(candidate.sessionId));
			auto clientSessionId = utility::conversions::to_utf8string(utility::conversions::to_base64(candidate.listeningPeerSessionId));

			logger->log(LogLevel::Debug, "p2p", "Starting P2P client connection client peer =" + std::to_string(candidate.clientPeer));

			auto connection = connections->getConnection(candidate.listeningPeer);
			if (connection && connection->getConnectionState() == ConnectionState::Connected)
			{
				sessions->updateSessionState(p2pSessionId, P2PSessionState::Connected);
				ctx->send([serializer](obytestream& stream)
				{
					serializer->serialize(stream, true);
				});
				return pplx::task_from_result();
			}

			logger->log(LogLevel::Debug, "p2p", "Connecting... ");
			connections->addPendingConnection(candidate.listeningPeer, clientSessionId)
				.then([sessions, p2pSessionId](std::shared_ptr<IConnection> connection)
			{
				connection->setMetadata("type", "p2p");
				sessions->updateSessionState(p2pSessionId, P2PSessionState::Connected);
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

			return transport->connect(candidate.listeningEndpointCandidate.address, p2pSessionId, ctx->packet()->connection->key())
				.then([logger, serializer, ctx](pplx::task<std::shared_ptr<IConnection>> t)
			{
				try
				{
					auto connection = t.get();
					logger->log(LogLevel::Trace, "p2p", "Successfully completed connection to " + std::to_string(connection->id()), "");
					ctx->send([serializer, connection](obytestream& stream)
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

		builder.service((byte)SystemRequestIDTypes::ID_P2P_OPEN_TUNNEL, [serializer, config, tunnels](std::shared_ptr<RequestContext> ctx)
		{
			auto serverId = serializer->deserializeOne<std::string>(ctx->inputStream());
			auto peerId = ctx->packet()->connection->id();
			if (!config->hasPublicIp())
			{
				auto strongTunnels = LockOrThrow(tunnels);
				auto handle = strongTunnels->addClient(serverId, peerId);

				ctx->send([serializer, handle](obytestream& stream)
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
				ctx->send([serializer, endpoint](obytestream& stream)
				{
					OpenTunnelResult result;
					result.useTunnel = false;
					result.endpoint = endpoint;
					serializer->serialize(stream, result);
				});
			}
			return pplx::task_from_result();
		});

		builder.service((byte)SystemRequestIDTypes::ID_P2P_CLOSE_TUNNEL, [serializer, tunnels](std::shared_ptr<RequestContext> ctx)
		{
			auto handle = serializer->deserializeOne<byte>(ctx->inputStream());
			LockOrThrow(tunnels)->closeTunnel(handle, ctx->packet()->connection->id());
			return pplx::task_from_result();
		});

		builder.service((byte)SystemRequestIDTypes::ID_P2P_RELAY_OPEN, [serializer, connections, sessions](std::shared_ptr<RequestContext> ctx)
		{
			auto relay = serializer->deserializeOne<OpenRelayParameters>(ctx->inputStream());
			auto p2pSessionId = utility::conversions::to_utf8string(utility::conversions::to_base64(relay.p2pSessionId));

			P2PSession p2pSession;
			if (!sessions->tryGetSession(p2pSessionId, p2pSession))
			{
				return pplx::task_from_exception<void>(std::runtime_error("P2PSession not found (" + p2pSessionId + ")"));
			}

			auto connection = std::make_shared<RelayConnection>(ctx->packet()->connection, relay.remotePeerAddress, relay.remotePeerId, p2pSessionId, serializer);
			static_cast<std::shared_ptr<IConnection>>(connection)->setSessionId(p2pSession.RemoteSessionId);
			connections->newConnection(connection);

			sessions->updateSessionState(p2pSessionId, P2PSessionState::Connected);
			ctx->send(StreamWriter());
			return pplx::task_from_result();
		});

		builder.service((byte)SystemRequestIDTypes::ID_P2P_RELAY_CLOSE, [serializer, connections](std::shared_ptr<RequestContext> ctx)
		{
			auto peerId = serializer->deserializeOne<uint64>(ctx->inputStream());

			auto connection = connections->getConnection(peerId);
			if (connection)
			{
				connections->closeConnection(connection, "");
			}
			ctx->send([](obytestream&) {});
			return pplx::task_from_result();
		});
	}
}

#include "stormancer/stdafx.h"
#include "stormancer/P2P/P2PRequestModule.h"
#include "stormancer/P2P/ConnectivityCandidate.h"
#include "stormancer/P2P/OpenRelayParameters.h"
#include "stormancer/P2P/RelayConnection.h"
#include "stormancer/SystemRequestIDTypes.h"
#include "stormancer/P2P/RakNet/P2PTunnels.h"
#include "stormancer/P2P/OpenTunnelResult.h"
#include "stormancer/Client.h"
#include "stormancer/Utilities/PointerUtilities.h"
#include "stormancer/P2P/P2PConnectToSceneMessage.h"
#include "stormancer/P2P/P2PDisconnectFromSceneMessage.h"
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
		std::shared_ptr<RequestProcessor> requestProcessor,
		std::weak_ptr<Client> wClient
	)
		: _transport(transport)
		, _connections(connections)
		, _sessions(sessions)
		, _serializer(serializer)
		, _tunnels(tunnels)
		, _logger(logger)
		, _config(config)
		, _requestProcessor(requestProcessor)
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
		std::weak_ptr<RequestProcessor> wRequestProcessor = _requestProcessor;
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

		builder.service((byte)SystemRequestIDTypes::ID_DISCONNECT_FROM_SCENE, [logger, serializer, wClient](std::shared_ptr<RequestContext> ctx) {
			P2PDisconnectFromSceneMessage disconnectRequest;
			serializer->deserialize(ctx->inputStream(), disconnectRequest);
			
			if (auto client = wClient.lock())
			{
				return client->getConnectedScene(disconnectRequest.sceneId)
					.then([ctx, wClient, disconnectRequest, serializer, logger](pplx::task<std::shared_ptr<Scene>> task)
				{
					std::shared_ptr<Scene> scene;
					try
					{
						scene = task.get();
					}
					catch (const std::exception& ex)
					{
						// Do nothing if the scene does not exist
						logger->log(LogLevel::Warn, "P2PRequestModule", "Scene not found while remote peer requests disconnection", ex.what());
					}

					if (scene)
					{
						std::static_pointer_cast<Scene_Impl>(scene)->raisePeerDisconnected(ctx->packet()->connection->sessionId());
					}

					P2PDisconnectFromSceneMessage disconnectResponse;
					disconnectResponse.sceneId = disconnectRequest.sceneId;
					disconnectResponse.reason = "Requested by remote peer";
					ctx->send([serializer, disconnectResponse](obytestream& stream)
					{
						serializer->serialize(stream, disconnectResponse);
					});
				});
			}
			else
			{
				// Do nothing if the client does not exist
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
			auto clientSessionId = utility::conversions::to_utf8string(utility::conversions::to_base64(candidate.listeningPeerSessionId));
			auto connection = connections->getConnection(clientSessionId);
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
			auto clientSessionId = utility::conversions::to_utf8string(utility::conversions::to_base64(candidate.clientPeerSessionId));
			auto connection = connections->getConnection(clientSessionId);
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

		auto registerConnectionOnCloseSession = [connections, wRequestProcessor, logger](std::shared_ptr<P2PSessions> p2pSessions, std::shared_ptr<IConnection> connection, std::string p2pSessionId, std::string parentId)
		{
			std::weak_ptr<IConnection> wHostConnection = connections->getConnection(parentId);

			bool p2pSessionFound = false;
			auto& p2pSession = p2pSessions->tryGetSession(p2pSessionId, p2pSessionFound);
			if (p2pSessionFound && !p2pSession.onConnectionCloseSubscription)
			{
				std::weak_ptr<P2PSessions> wP2pSessions = p2pSessions;
				p2pSession.onConnectionCloseSubscription = connection->onClose.subscribe([wP2pSessions, p2pSessionId, wHostConnection, wRequestProcessor, logger](std::string reason)
				{
					auto hostConnection = wHostConnection.lock();
					auto requestProcessor = wRequestProcessor.lock();
					if (hostConnection && requestProcessor)
					{
						auto p2pSessonIdVector = utility::conversions::from_base64(utility::conversions::to_string_t(p2pSessionId));
						logger->log(LogLevel::Trace, "P2PRequestModule", "Closing P2P session", p2pSessionId);
						requestProcessor->sendSystemRequest(hostConnection.get(), (byte)SystemRequestIDTypes::ID_P2P_CLOSE_SESSION, p2pSessonIdVector, timeout(5s))
							.then([logger](pplx::task<void> task)
						{
							try
							{
								task.get();
							}
							catch (const std::exception& ex)
							{
								logger->log(LogLevel::Warn, "P2PRequestModule", "Send P2P Close session failed", ex.what());
							}
						});
					}
					if (auto p2pSessions = wP2pSessions.lock())
					{
						p2pSessions->closeSession(p2pSessionId);
					}
				});
			}
		};

		builder.service((byte)SystemRequestIDTypes::ID_P2P_CONNECT_HOST, [serializer, connections, sessions, logger, registerConnectionOnCloseSession](std::shared_ptr<RequestContext> ctx)
		{
			auto candidate = serializer->deserializeOne<ConnectivityCandidate>(ctx->inputStream());

			auto p2pSessionId = utility::conversions::to_utf8string(utility::conversions::to_base64(candidate.p2pSessionId));
			auto clientSessionId = utility::conversions::to_utf8string(utility::conversions::to_base64(candidate.clientPeerSessionId));
			auto parentId = ctx->packet()->connection->key();

			auto connection = connections->getConnection(clientSessionId);
			if (connection && connection->getConnectionState() == ConnectionState::Connected)
			{
				sessions->updateSessionState(p2pSessionId, P2PSessionState::Connected);
				ctx->send(StreamWriter());
				registerConnectionOnCloseSession(sessions, connection, p2pSessionId, parentId);
				return pplx::task_from_result();
			}

			logger->log(LogLevel::Debug, "p2p", "Waiting connection ");
			connections->addPendingConnection(clientSessionId)
				.then([sessions, p2pSessionId, registerConnectionOnCloseSession, parentId](std::shared_ptr<IConnection> connection)
			{
				connection->setMetadata("type", "p2p");
				sessions->updateSessionState(p2pSessionId, P2PSessionState::Connected);
				registerConnectionOnCloseSession(sessions, connection, p2pSessionId, parentId);
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

		builder.service((byte)SystemRequestIDTypes::ID_P2P_CONNECT_CLIENT, [serializer, logger, connections, sessions, transport, registerConnectionOnCloseSession](std::shared_ptr<RequestContext> ctx)
		{
			auto candidate = serializer->deserializeOne<ConnectivityCandidate>(ctx->inputStream());

			auto p2pSessionId = utility::conversions::to_utf8string(utility::conversions::to_base64(candidate.p2pSessionId));
			auto clientSessionId = utility::conversions::to_utf8string(utility::conversions::to_base64(candidate.listeningPeerSessionId));
			auto parentId = ctx->packet()->connection->key();

			logger->log(LogLevel::Debug, "p2p", "Starting P2P client connection client peer", std::to_string(candidate.clientPeer));

			auto connection = connections->getConnection(clientSessionId);
			if (connection && connection->getConnectionState() == ConnectionState::Connected)
			{
				sessions->updateSessionState(p2pSessionId, P2PSessionState::Connected);
				ctx->send([serializer](obytestream& stream)
				{
					serializer->serialize(stream, true);
				});
				registerConnectionOnCloseSession(sessions, connection, p2pSessionId, parentId);
				return pplx::task_from_result();
			}

			logger->log(LogLevel::Debug, "p2p", "Connecting... ");
			connections->addPendingConnection( clientSessionId)
				.then([sessions, p2pSessionId, registerConnectionOnCloseSession, parentId](std::shared_ptr<IConnection> connection)
			{
				connection->setMetadata("type", "p2p");
				sessions->updateSessionState(p2pSessionId, P2PSessionState::Connected);
				registerConnectionOnCloseSession(sessions, connection, p2pSessionId, parentId);
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

			return transport->connect(candidate.listeningEndpointCandidate.address, p2pSessionId, parentId)
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
			auto peerId = ctx->packet()->connection->sessionId();
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
			LockOrThrow(tunnels)->closeTunnel(handle, ctx->packet()->connection->sessionId());
			return pplx::task_from_result();
		});

		builder.service((byte)SystemRequestIDTypes::ID_P2P_RELAY_OPEN, [serializer, connections, sessions](std::shared_ptr<RequestContext> ctx)
		{
			auto relay = serializer->deserializeOne<OpenRelayParameters>(ctx->inputStream());
			auto p2pSessionId = utility::conversions::to_utf8string(utility::conversions::to_base64(relay.p2pSessionId));

			bool sessionFound = false;
			auto& p2pSession = sessions->tryGetSession(p2pSessionId, sessionFound);
			if (!sessionFound)
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
			auto peerId = serializer->deserializeOne<std::string>(ctx->inputStream());

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

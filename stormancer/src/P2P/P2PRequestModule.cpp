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
	{
		_transport = transport;
		_connections = connections;
		_sessions = sessions;
		_serializer = serializer;
		_tunnels = tunnels;
		_logger = logger;
		_config = config;
	}
	
	void P2PRequestModule::registerModule(RequestModuleBuilder* builder)
	{
		builder->service((byte)SystemRequestIDTypes::ID_P2P_GATHER_IP, [=](RequestContext* ctx) {
			std::vector<std::string> endpoints;
			if (_config->dedicatedServerEndpoint != "")
			{
				endpoints.push_back(_config->dedicatedServerEndpoint + ":" + std::to_string(_config->clientSDKPort));
			}
			else if (_config->enableNatPunchthrough == false)
			{
				endpoints.clear();
			}
			else
			{
				endpoints = _transport->getAvailableEndpoints();
			}
			ctx->send([=](obytestream* stream) {
				_serializer->serialize(stream, endpoints);
			});
			return pplx::task_from_result();
		});

		builder->service((byte)SystemRequestIDTypes::ID_P2P_CREATE_SESSION, [=](RequestContext* ctx) {
			auto sessionIdVector = _serializer->deserializeOne<std::vector<char>>(ctx->inputStream());
			std::string sessionId(sessionIdVector.begin(), sessionIdVector.end());
			auto peerId = _serializer->deserializeOne<uint64>(ctx->inputStream());

			//TODO : use true scene id when it's available from the server
			std::string sceneId = "";
			//auto sceneId = _serializer->deserializeOne<std::string>(ctx->inputStream());			
			P2PSession session;
			session.sessionId = sessionIdVector;
			session.sceneId = sceneId;
			session.remotePeer = peerId;
			_sessions->createSession(sessionId, session);
			ctx->send(Writer());
			return pplx::task_from_result();
		});

		builder->service((byte)SystemRequestIDTypes::ID_P2P_CLOSE_SESSION, [=](RequestContext* ctx) {

			auto sessionIdVector = _serializer->deserializeOne<std::vector<byte>>(ctx->inputStream());
			std::string sessionId(sessionIdVector.begin(), sessionIdVector.end());
			_sessions->closeSession(sessionId);
			ctx->send(Writer());

			return pplx::task_from_result();
		});

		builder->service((byte)SystemRequestIDTypes::ID_P2P_TEST_CONNECTIVITY_CLIENT, [=](RequestContext* ctx) {
			auto candidate = _serializer->deserializeOne<ConnectivityCandidate>(ctx->inputStream());
			_logger->log(LogLevel::Debug, "p2p", "Starting connectivity test (CLIENT) " + candidate.clientEndpointCandidate.address + " => " + candidate.listeningEndpointCandidate.address);
			auto connection = _connections->getConnection(candidate.listeningPeer);
			if (connection && connection->getConnectionState() == ConnectionState::Connected)
			{
				ctx->send([=](obytestream* s) {
					_serializer->serialize(s, 0);
				});
				return pplx::task_from_result();
			}
			if (_config->enableNatPunchthrough == false)
			{
				if (candidate.listeningEndpointCandidate.address.substr(0, 9) == "127.0.0.1")
				{
					ctx->send([=](obytestream* stream) {
						_serializer->serialize(stream, -1);
					});
					return pplx::task_from_result();
				}
				else
				{
					ctx->send([=](obytestream* stream) {
						_serializer->serialize(stream, 1);
					});
					return pplx::task_from_result();
				}
			}
			else if (_config->dedicatedServerEndpoint != "")
			{
				ctx->send([=](obytestream* stream) {
					_serializer->serialize(stream, -1);
				});
				return pplx::task_from_result();
			}
			else
			{
				return _transport->sendPing(candidate.listeningEndpointCandidate.address).then([=](pplx::task<int> t) {
					auto latency = (int)t.get();
					_logger->log(LogLevel::Debug, "p2p", "Connectivity test complete : " + candidate.clientEndpointCandidate.address + " => " + candidate.listeningEndpointCandidate.address + " ping : " + std::to_string(latency));
					ctx->send([=](obytestream* stream) {
						_serializer->serialize(stream, latency);
					});
				});
			}
		});

		builder->service((byte)SystemRequestIDTypes::ID_P2P_TEST_CONNECTIVITY_HOST, [=](RequestContext* ctx) {
			auto candidate = _serializer->deserializeOne<ConnectivityCandidate>(ctx->inputStream());
			_logger->log(LogLevel::Debug, "p2p", "Starting connectivity test (LISTENER) " + candidate.clientEndpointCandidate.address + " => " + candidate.listeningEndpointCandidate.address);
			auto connection = _connections->getConnection(candidate.clientPeer);
			if (connection && connection->getConnectionState() == ConnectionState::Connected)
			{
				ctx->send(Writer());
				return pplx::task_from_result();
			}
			if (_config->dedicatedServerEndpoint == "" && _config->enableNatPunchthrough)
			{
				_transport->openNat(candidate.clientEndpointCandidate.address);
			}
			ctx->send(Writer());
			return pplx::task_from_result();
		});

		builder->service((byte)SystemRequestIDTypes::ID_P2P_CONNECT_HOST, [=](RequestContext* ctx) {
			auto candidate = _serializer->deserializeOne<ConnectivityCandidate>(ctx->inputStream());
			std::string sessionId(candidate.sessionId.begin(), candidate.sessionId.end());

			auto connection = _connections->getConnection(candidate.clientPeer);
			if (connection && connection->getConnectionState() == ConnectionState::Connected)
			{
				_sessions->updateSessionState(sessionId, P2PSessionState::Connected);
				ctx->send(Writer());
				return pplx::task_from_result();
			}

			_logger->log(LogLevel::Debug, "p2p", "Waiting connection " + candidate.clientEndpointCandidate.address + " => " + candidate.listeningEndpointCandidate.address);
			_connections->addPendingConnection(candidate.clientPeer)
				.then([=](pplx::task<std::shared_ptr<IConnection>> t) {
				_sessions->updateSessionState(sessionId, P2PSessionState::Connected);
			});

			ctx->send(Writer());
			return pplx::task_from_result();
		});

		builder->service((byte)SystemRequestIDTypes::ID_P2P_CONNECT_CLIENT, [=](RequestContext* ctx) {
			auto candidate = _serializer->deserializeOne<ConnectivityCandidate>(ctx->inputStream());
			std::string sessionId(candidate.sessionId.begin(), candidate.sessionId.end());

			_logger->log(LogLevel::Debug, "p2p", "Starting P2P client connection client peer =" + std::to_string(candidate.clientPeer));

			auto connection = _connections->getConnection(candidate.listeningPeer);
			if (connection && connection->getConnectionState() == ConnectionState::Connected)
			{
				_sessions->updateSessionState(sessionId, P2PSessionState::Connected);
				ctx->send([=](obytestream* stream) {
					_serializer->serialize(stream, true);
				});
				return pplx::task_from_result();
			}

			_logger->log(LogLevel::Debug, "p2p", "Connecting... " + candidate.clientEndpointCandidate.address + " => " + candidate.listeningEndpointCandidate.address);
			_connections->addPendingConnection(candidate.listeningPeer)
				.then([=](pplx::task<std::shared_ptr<IConnection>> t) {

				_sessions->updateSessionState(sessionId, P2PSessionState::Connected);
			});

			return _transport->connect(candidate.listeningEndpointCandidate.address)
				.then([=](pplx::task<std::shared_ptr<IConnection>> t) {
				try
				{
					auto connection = t.get();
					_logger->log(LogLevel::Trace, "p2p", "Successfully completed connection to " + std::to_string(connection->id()), "");
					ctx->send([=](obytestream* stream) {
						bool connected = connection && connection->getConnectionState() == ConnectionState::Connected;
						_serializer->serialize(stream, connected);
					});
				}
				catch (const std::exception& ex)
				{
					_logger->log(LogLevel::Error, "p2p", "Connection attempt failed: ", ex.what());
					throw;
				}
				
			});
		});

		builder->service((byte)SystemRequestIDTypes::ID_P2P_OPEN_TUNNEL, [=](RequestContext* ctx) {
			auto serverId = _serializer->deserializeOne<std::string>(ctx->inputStream());
			auto peerId = ctx->packet()->connection->id();
			if (!_config->hasPublicIp())
			{
				auto handle = _tunnels->addClient(serverId, peerId);

				ctx->send([=](obytestream* stream) {
					OpenTunnelResult result;
					result.useTunnel = true;
					result.handle = handle;
					_serializer->serialize(stream, result);
					
				});
			}
			else
			{
				std::string endpoint = _config->getIp_Port();//Dedicated server IP:port
				ctx->send([endpoint,this](obytestream* stream) {
					OpenTunnelResult result;
					result.useTunnel = false;
					result.endpoint = endpoint;
					_serializer->serialize(stream, result);
				});
			}
			return pplx::task_from_result();

		});

		builder->service((byte)SystemRequestIDTypes::ID_P2P_CLOSE_TUNNEL, [=](RequestContext* ctx) {
			auto handle = _serializer->deserializeOne<byte>(ctx->inputStream());
			_tunnels->closeTunnel(handle, ctx->packet()->connection->id());
			return pplx::task_from_result();
		});

		builder->service((byte)SystemRequestIDTypes::ID_P2P_RELAY_OPEN, [=](RequestContext* ctx) {
			auto relay = _serializer->deserializeOne<OpenRelayParameters>(ctx->inputStream());

			_connections->newConnection(std::make_shared<RelayConnection>(_connections->getConnection(0), relay.remotePeerAddress, relay.remotePeerId));
			std::string sessionId(relay.sessionId.begin(), relay.sessionId.end());
			_sessions->updateSessionState(sessionId, P2PSessionState::Connected);
			ctx->send(Writer());
			return pplx::task_from_result();
		});

		builder->service((byte)SystemRequestIDTypes::ID_P2P_RELAY_CLOSE, [=](RequestContext* ctx) {
			auto peerId = _serializer->deserializeOne<uint64>(ctx->inputStream());
			auto connection = _connections->getConnection(peerId);
			if (connection)
			{
				_connections->closeConnection(connection, "");
			}
			ctx->send([=](obytestream*) {

			});
			return pplx::task_from_result();
		});
	}
};

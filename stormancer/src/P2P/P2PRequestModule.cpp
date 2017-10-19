#include "stdafx.h"
#include "P2P/P2PRequestModule.h"
#include "P2P/ConnectivityCandidate.h"
#include "P2P/OpenRelayParameters.h"
#include "P2P/RelayConnection.h"
#include "SystemRequestIDTypes.h"
#include "P2P/RakNet/P2PTunnels.h"

namespace Stormancer
{
	P2PRequestModule::P2PRequestModule(std::shared_ptr<ITransport> transport, std::shared_ptr<IConnectionManager> connections, std::shared_ptr<P2PSessions> sessions, std::shared_ptr<Serializer> serializer, std::shared_ptr<P2PTunnels> tunnels, std::shared_ptr<ILogger> logger)
	{
		_transport = transport;
		_connections = connections;
		_sessions = sessions;
		_serializer = serializer;
		_tunnels = tunnels;
		_logger = logger;
	}

	void P2PRequestModule::registerModule(RequestModuleBuilder* builder)
	{
		builder->service((byte)SystemRequestIDTypes::ID_P2P_GATHER_IP, [this](RequestContext* ctx) {
			auto endpoints = _transport->getAvailableEndpoints();
			ctx->send([this, endpoints](bytestream* s) {
				_serializer->serialize(endpoints, s);
			});
			return pplx::task_from_result();
		});

		builder->service((byte)SystemRequestIDTypes::ID_P2P_CREATE_SESSION, [this](RequestContext* ctx) {
			auto sessionIdVector = _serializer->deserialize<std::vector<char>>(ctx->inputStream());
			std::string sessionId(sessionIdVector.begin(), sessionIdVector.end());
			auto peerId = _serializer->deserialize<uint64>(ctx->inputStream());

			//TODO : use true scene id when it's available from the server
			std::string sceneId = "";
			//auto sceneId = _serializer->deserialize<std::string>(ctx->inputStream());			
			P2PSession session;
			session.sessionId = sessionIdVector;
			session.sceneId = sceneId;
			session.remotePeer = peerId;
			_sessions->createSession(sessionId, session);
			ctx->send([](bytestream*) {});
			return pplx::task_from_result();
		});

		builder->service((byte)SystemRequestIDTypes::ID_P2P_CLOSE_SESSION, [this](RequestContext* ctx) {

			auto sessionIdVector = _serializer->deserialize<std::vector<byte>>(ctx->inputStream());
			std::string sessionId(sessionIdVector.begin(), sessionIdVector.end());
			_sessions->closeSession(sessionId);
			ctx->send([](bytestream*) {
				return;
			});

			return pplx::task_from_result();
		});

		builder->service((byte)SystemRequestIDTypes::ID_P2P_TEST_CONNECTIVITY_CLIENT, [this](RequestContext* ctx) {
			auto candidate = _serializer->deserialize<ConnectivityCandidate>(ctx->inputStream());
			_logger->log(LogLevel::Debug, "p2p", "Starting connectivity test (CLIENT) " + candidate.clientEndpointCandidate.address + " => " + candidate.listeningEndpointCandidate.address);
			/*auto connection = _connections->getConnection(candidate.listeningPeer);
			if (connection && connection->getConnectionState() == ConnectionState::Connected)
			{
				ctx->send([this](bytestream* s) {
					_serializer->serialize(0, s);
				});
				return pplx::task_from_result();
			}*/
			
			return _transport->sendPing(candidate.listeningEndpointCandidate.address).then([this, ctx, candidate](pplx::task<int> t) {
				auto latency = (int)t.get();
				_logger->log(LogLevel::Debug, "p2p", "Connectivity test complete : " + candidate.clientEndpointCandidate.address + " => " + candidate.listeningEndpointCandidate.address + " ping : " + std::to_string(latency));
				ctx->send([this, latency](bytestream* s) {

					_serializer->serialize(latency, s);
				});
			});


		});

		builder->service((byte)SystemRequestIDTypes::ID_P2P_TEST_CONNECTIVITY_HOST, [this](RequestContext* ctx) {
			auto candidate = _serializer->deserialize<ConnectivityCandidate>(ctx->inputStream());
			_logger->log(LogLevel::Debug, "p2p", "Starting connectivity test (LISTENER) " + candidate.clientEndpointCandidate.address + " => " + candidate.listeningEndpointCandidate.address);
			/*auto connection = _connections->getConnection(candidate.clientPeer);
			if (connection && connection->getConnectionState() == ConnectionState::Connected)
			{
				ctx->send([this](bytestream* s) {
				});
				return pplx::task_from_result();
			}*/

			_transport->openNat(candidate.clientEndpointCandidate.address);
			ctx->send([](bytestream*) {});
			return pplx::task_from_result();
		});

		builder->service((byte)SystemRequestIDTypes::ID_P2P_CONNECT_HOST, [this](RequestContext* ctx) {
			auto candidate = _serializer->deserialize<ConnectivityCandidate>(ctx->inputStream());
			
			auto connection = _connections->getConnection(candidate.clientPeer);
			if (connection && connection->getConnectionState() == ConnectionState::Connected)
			{
				ctx->send([](bytestream*) {});
				return pplx::task_from_result();
			}
			
			_logger->log(LogLevel::Debug, "p2p", "Waiting connection " + candidate.clientEndpointCandidate.address + " => " + candidate.listeningEndpointCandidate.address);
			_connections->addPendingConnection(candidate.clientPeer)
				.then([this, candidate](pplx::task<std::shared_ptr<IConnection>> t) {

				std::string sessionId(candidate.sessionId.begin(), candidate.sessionId.end());
				_sessions->updateSessionState(sessionId, P2PSessionState::Connected);
			});

			ctx->send([](bytestream*) {});
			return pplx::task_from_result();
		});

		builder->service((byte)SystemRequestIDTypes::ID_P2P_CONNECT_CLIENT, [this](RequestContext* ctx) {
			auto candidate = _serializer->deserialize<ConnectivityCandidate>(ctx->inputStream());
			
			auto connection = _connections->getConnection(candidate.listeningPeer);
			if (connection && connection->getConnectionState() == ConnectionState::Connected)
			{
				ctx->send([this](bytestream* s) {
					_serializer->serialize(true, s);
				});
				return pplx::task_from_result();
			}
			
			_logger->log(LogLevel::Debug, "p2p", "Connecting... " + candidate.clientEndpointCandidate.address + " => " + candidate.listeningEndpointCandidate.address);
			_connections->addPendingConnection(candidate.listeningPeer)
				.then([this, candidate](pplx::task<std::shared_ptr<IConnection>> t) {

				std::string sessionId(candidate.sessionId.begin(), candidate.sessionId.end());
				_sessions->updateSessionState(sessionId, P2PSessionState::Connected);
			});

			return _transport->connect(candidate.listeningEndpointCandidate.address)
				.then([this, candidate, ctx](pplx::task<std::shared_ptr<IConnection>> t) {

				auto connection = t.get();
				ctx->send([this, connection](bytestream* s) {
					_serializer->serialize(connection && connection->getConnectionState() == ConnectionState::Connected, s);
				});
			});


		});
		builder->service((byte)SystemRequestIDTypes::ID_P2P_OPEN_TUNNEL, [this](RequestContext* ctx) {
			auto serverId = _serializer->deserialize<std::string>(ctx->inputStream());
			auto peerId = ctx->packet()->connection->id();
			auto handle = _tunnels->addClient(serverId, peerId);

			ctx->send([this, handle](bytestream* s) {
				_serializer->serialize(handle, s);
			});
			return pplx::task_from_result();

		});

		builder->service((byte)SystemRequestIDTypes::ID_P2P_CLOSE_TUNNEL, [this](RequestContext* ctx) {
			auto handle = _serializer->deserialize<byte>(ctx->inputStream());
			_tunnels->closeTunnel(handle, ctx->packet()->connection->id());
			return pplx::task_from_result();
		});
		builder->service((byte)SystemRequestIDTypes::ID_P2P_RELAY_OPEN, [this](RequestContext* ctx) {
			auto relay = _serializer->deserialize<OpenRelayParameters>(ctx->inputStream());

			_connections->newConnection(std::make_shared<RelayConnection>(_connections->getConnection(0), relay.remotePeerAddress, relay.remotePeerId));
			std::string sessionId(relay.sessionId.begin(), relay.sessionId.end());
			_sessions->updateSessionState(sessionId, P2PSessionState::Connected);
			ctx->send([this](bytestream*) {

			});
			return pplx::task_from_result();
		});
		builder->service((byte)SystemRequestIDTypes::ID_P2P_RELAY_CLOSE, [this](RequestContext* ctx) {
			auto peerId = _serializer->deserialize<uint64>(ctx->inputStream());
			auto connection = _connections->getConnection(peerId);
			if (connection)
			{
				_connections->closeConnection(connection, "");
			}
			ctx->send([this](bytestream*) {

			});
			return pplx::task_from_result();
		});
	}

};

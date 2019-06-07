#pragma once

#include "stormancer/BuildConfig.h"
#include "stormancer/MessageOriginFilter.h"
#include "stormancer/Route.h"
#include "stormancer/PeerFilter.h"
#include "stormancer/P2P/P2PConnectionStateChangedArgs.h"
#include "stormancer/P2P/IP2PScenePeer.h"
#include "stormancer/P2P/P2PConnectToSceneMessage.h"
#include "stormancer/ConnectionState.h"
#include "stormancer/Packet.h"
#include "stormancer/DependencyInjection.h"
#include "stormancer/Tasks.h"
#include "stormancer/Event.h"
#include "rxcpp/rx.hpp"
#include <memory>
#include <unordered_map>

namespace Stormancer
{
	class P2PService;

	struct SceneAddress
	{
		///urn format : either {sceneI} or scene:/{sceneId} scene:/{app}/{sceneId} , scene:/{account}/{app}/{sceneId} or scene:/{cluster}/{account}/{app}/{sceneId}
		static SceneAddress  parse(const std::string urn, const std::string& defaultClusterId, const std::string& defaultAccount, const std::string& defaultApp);
		std::string clusterId;
		std::string app;
		std::string account;
		std::string sceneId;

		std::string toUri() const
		{
			return "scene:/" + clusterId + "/" + account + "/" + app + "/" + sceneId;
		}
	};


	class Scene
	{
	public:

		virtual ~Scene() = default;

		/// Disconnect from the scene.
		virtual pplx::task<void> disconnect() = 0;

		/// Add a route to the scene.
		/// \param routeName Route name.
		/// \param handler Function which handle the receiving messages from the server on the route.
		/// \param metadata Metadatas about the Route.
		/// Add a route for each different message type.
		virtual void addRoute(
			const std::string& routeName,
			std::function<void(Packetisp_ptr)> handler,
			MessageOriginFilter origin = MessageOriginFilter::Host,
			const std::unordered_map<std::string, std::string>& metadata = std::unordered_map<std::string, std::string>()
		) = 0;

		/// Send a packet to a route.
		/// \param peerFilter Peer receiver.
		/// \param routeName Route name.
		/// \param streamWriter Function where we write the data in the byte stream.
		/// \param priority Message priority on the network.
		/// \param reliability Message reliability behavior.
		virtual void send(
			const PeerFilter& peerFilter,
			const std::string& routeName,
			const StreamWriter& streamWriter,
			PacketPriority priority = PacketPriority::MEDIUM_PRIORITY,
			PacketReliability reliability = PacketReliability::RELIABLE_ORDERED,
			const std::string& channelIdentifier = ""
		) = 0;

		/// Send a packet to a route.
		/// \param routeName Route name.
		/// \param streamWriter Function where we write the data in the byte stream.
		/// \param priority Message priority on the network.
		/// \param reliability Message reliability behavior.
		virtual void send(
			const std::string& routeName,
			const StreamWriter& streamWriter,
			PacketPriority priority = PacketPriority::MEDIUM_PRIORITY,
			PacketReliability reliability = PacketReliability::RELIABLE_ORDERED,
			const std::string& channelIdentifier = ""
		) = 0;

		/// Returns the connection state to the the scene.
		virtual ConnectionState getCurrentConnectionState() const = 0;

		virtual rxcpp::observable<ConnectionState> getConnectionStateChangedObservable() const = 0;

		/// Returns the scene id.
		virtual std::string id() const = 0;

		/// Returns the detailed scene address, including cluster, account and app information
		virtual SceneAddress address() const = 0;

		/// Returns the scene handle.
		virtual byte handle() const = 0;

		/// Returns a host metadata value.
		virtual std::string getHostMetadata(const std::string& key) const = 0;

		/// Returns the host connection.
		virtual std::weak_ptr<IConnection> hostConnection() const = 0;

		/// Returns a copy of the local routes.
		virtual std::vector<Route_ptr> localRoutes() const = 0;

		/// Returns a copy of the remote routes.
		virtual std::vector<Route_ptr> remoteRoutes() const = 0;

		/// Creates an IObservable<Packet> instance that listen to events on the specified route.
		/// \param A string containing the name of the route to listen to.
		/// \return An IObservable<Packet> instance that fires each time a message is received on the route.
		virtual rxcpp::observable<Packetisp_ptr> onMessage(const std::string& routeName, MessageOriginFilter filter = MessageOriginFilter::Host, const std::unordered_map<std::string, std::string>& metadata = std::unordered_map<std::string, std::string>()) = 0;

		/// Get a vector containing the scene host connections.
		/// \return A vector containing the scene host connections.
		virtual std::vector<std::shared_ptr<IScenePeer>> remotePeers() const = 0;

		/// Used by stormancer to add a P2P scene peer to the connected peers (don't use it manually)
		virtual std::shared_ptr<IP2PScenePeer> peerConnected(std::shared_ptr<IConnection> connection, std::shared_ptr<P2PService> P2P, P2PConnectToSceneMessage) = 0;

		/// Returns the peer connection to the host.
		virtual std::shared_ptr<IScenePeer> host() const = 0;

		virtual const DependencyScope& dependencyResolver() const = 0;

		/// Fire when a packet is received in the scene. 
		virtual Event<Packet_ptr> onPacketReceived() = 0;

		virtual bool isHost() const = 0;

		virtual std::unordered_map<uint64, std::shared_ptr<IP2PScenePeer>> connectedPeers() const = 0;

		virtual Event<std::shared_ptr<IP2PScenePeer>> onPeerConnected() const = 0;

		virtual rxcpp::observable<P2PConnectionStateChangedArgs> p2pConnectionStateChanged() const = 0;

		virtual std::shared_ptr<P2PTunnel> registerP2PServer(const std::string& p2pServerId) = 0;

		virtual pplx::task<std::shared_ptr<IP2PScenePeer>> openP2PConnection(const std::string& p2pToken, pplx::cancellation_token ct = pplx::cancellation_token::none()) = 0;

		virtual const std::unordered_map<std::string, std::string>& getSceneMetadata() = 0;
	};
}

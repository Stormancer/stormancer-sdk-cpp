#pragma once

#include "stormancer/BuildConfig.h"
#include "stormancer/Scene.h"
#include "stormancer/Streams/bytestream.h"
#include "stormancer/PacketPriority.h"
#include "stormancer/ConnectionResult.h"
#include "stormancer/SceneInfosDto.h"
#include "stormancer/Route.h"
#include "stormancer/IScenePeer.h"
#include "stormancer/Packet.h"
#include "stormancer/PeerFilter.h"
#include "stormancer/P2P/P2PTunnel.h"
#include "stormancer/P2P/P2PScenePeer.h"
#include "stormancer/P2P/P2PConnectionStateChangedArgs.h"
#include "stormancer/DependencyInjection.h"

#include <mutex>

namespace Stormancer
{
	class Client;
	class Scene_Impl;
	class IPlugin;
	
	/// Communicates with the server scene.
	/// Get a scene by calling Client::getPublicScene or Client::getScene.
	class STORMANCER_DLL_API Scene_Impl : public std::enable_shared_from_this<Scene_Impl>, public Scene
	{
	public:

		/// The Client need to access some private members of the Scene.
		friend class Client;

		/// The SceneDispatcher need to access some private members of the Scene.
		friend class SceneDispatcher;

		/// The P2PScenePeer has to access to raisePeerDisconnected
		friend class P2PScenePeer;

		/// The P2PRequestModule has to access to raisePeerConnected
		friend class P2PRequestModule;

#pragma region public_methods

		/// Connect to the scene.
		pplx::task<void> connect(pplx::cancellation_token ct = pplx::cancellation_token::none());

		/// Disconnect from the scene.
		pplx::task<void> disconnect(pplx::cancellation_token ct = pplx::cancellation_token::none()) override;

		/// Add a route to the scene.
		/// \param routeName Route name.
		/// \param handler Function which handle the receiving messages from the server on the route.
		/// \param metadata Metadatas about the Route.
		/// Add a route for each different message type.
		void addRoute(const std::string& routeName, std::function<void(Packetisp_ptr)> handler, MessageOriginFilter origin = MessageOriginFilter::Host, const std::unordered_map<std::string, std::string>& metadata = std::unordered_map<std::string, std::string>()) override;

		/// Send a packet to a route.
		/// \param peerFilter Peer receiver.
		/// \param routeName Route name.
		/// \param streamWriter Function where we write the data in the byte stream.
		/// \param priority Message priority on the network.
		/// \param reliability Message reliability behavior.
		void send(const PeerFilter& peerFilter, const std::string& routeName, const StreamWriter& streamWriter, PacketPriority priority = PacketPriority::MEDIUM_PRIORITY, PacketReliability reliability = PacketReliability::RELIABLE_ORDERED, const std::string& channelIdentifier = "") override;

		/// Send a packet to a route.
		/// \param routeName Route name.
		/// \param streamWriter Function where we write the data in the byte stream.
		/// \param priority Message priority on the network.
		/// \param reliability Message reliability behavior.
		void send(const std::string& routeName, const StreamWriter& streamWriter, PacketPriority priority = PacketPriority::MEDIUM_PRIORITY, PacketReliability reliability = PacketReliability::RELIABLE_ORDERED, const std::string& channelIdentifier = "") override;

		/// Returns the connection state to the the scene.
		ConnectionState getCurrentConnectionState() const override;

		rxcpp::observable<ConnectionState> getConnectionStateChangedObservable() const override;

		/// Returns the scene id.
		std::string id() const override;
		
		// Returns the detailed scene address, including cluster, account and app information
		SceneAddress address() const override;

		/// Returns a host metadata value.
		std::string getHostMetadata(const std::string& key) const override;

		/// Returns the host connection.
		std::weak_ptr<IConnection> hostConnection() const override;

		/// Returns a copy of the local routes.
		std::vector<Route_ptr> localRoutes() const override;

		/// Returns a copy of the remote routes.
		std::vector<Route_ptr> remoteRoutes() const override;

		/// Creates an IObservable<Packet> instance that listen to events on the specified route.
		/// \param A string containing the name of the route to listen to.
		/// \return An IObservable<Packet> instance that fires each time a message is received on the route.
		rxcpp::observable<Packetisp_ptr> onMessage(const std::string& routeName, MessageOriginFilter filter = MessageOriginFilter::Host, const std::unordered_map<std::string, std::string>& metadata = std::unordered_map<std::string, std::string>()) override;

		/// Get a vector containing the scene host connections.
		/// \return A vector containing the scene host connections.
		std::vector<std::shared_ptr<IScenePeer>> remotePeers() const override;

		/// Returns the peer connection to the host.
		std::shared_ptr<IScenePeer> host() const override;

		const DependencyScope& dependencyResolver() const override;

		/// Fire when a packet is received in the scene. 
		Event<Packet_ptr> onPacketReceived() override;

		bool isHost() const override;

		const std::unordered_map<std::string, std::shared_ptr<IP2PScenePeer>>& connectedPeers() const override;

		Event<std::shared_ptr<IP2PScenePeer>> onPeerConnected() const override;

		Event<std::shared_ptr<IP2PScenePeer>> onPeerDisconnected() const override;

		rxcpp::observable<P2PConnectionStateChangedArgs> p2pConnectionStateChanged() const override;

		std::shared_ptr<P2PTunnel> registerP2PServer(const std::string& p2pServerId) override;

		pplx::task<std::shared_ptr<IP2PScenePeer>> openP2PConnection(const std::string& p2pToken, pplx::cancellation_token ct = pplx::cancellation_token::none()) override;

		const std::unordered_map<std::string, std::string>& getSceneMetadata() override;

		std::shared_ptr<IP2PScenePeer> peerConnected(std::shared_ptr<IConnection> connection, std::shared_ptr<P2PService> P2P, P2PConnectToSceneMessage);

		std::weak_ptr<IConnection> hostConnection();

#pragma endregion

	private:

#pragma region private_methods

		/// Constructor.
		/// \param connection The connection to the scene.
		/// \param client The client which manage the connection.
		/// \param id Scene id.
		/// \param token Application token.
		/// \param dto Scene informations.
		Scene_Impl(std::weak_ptr<IConnection> connection, std::weak_ptr<Client> client, const SceneAddress& address, const std::string& token, const SceneInfosDto& dto, DependencyScope& parentScope, std::vector<IPlugin*>& plugins);

		/// Copy constructor deleted.
		Scene_Impl(const Scene& other) = delete;

		/// Move constructor deleted.
		Scene_Impl(const Scene&& other) = delete;

		/// Destructor.
		~Scene_Impl();

		/// Copy operator deleted.
		Scene_Impl& operator=(const Scene_Impl& other) = delete;

		void send(std::shared_ptr<IScenePeer> scenePeer, const std::string& routeName, const StreamWriter& streamWriter, PacketPriority priority = PacketPriority::MEDIUM_PRIORITY, PacketReliability reliability = PacketReliability::RELIABLE_ORDERED, const std::string& channelIdentifier = "");

		/// Creates an IObservable<Packet> instance that listen to events on the specified route.
		/// \param route A pointer of the Route instance to listen to.
		/// \return An IObservable<Packet> instance that fires each time a message is received on the route.
		rxcpp::observable<Packetisp_ptr> onMessage(Route_ptr route);

		/// Finalize the connection to the scene.
		/// \param cr Connection result message retrieved by a system request.
		void completeConnectionInitialization(ConnectionResult& cr);

		/// Handle a message received on the scene and dispatch the packet to the right route handle.
		/// \param packet Receivedpacket.
		void handleMessage(Packet_ptr packet);

		/// <summary>
		/// Change the scene's connection state.
		/// </summary>
		/// <remarks>
		/// Trigger the relevant plugin handlers and notify the connection state observers.
		/// No effect if <c>connectionState == getCurrentConnectionState()</c>.
		/// </remarks>
		/// <param name="connectionState">The new connection state.</param>
		/// <param name="async">
		/// Whether the connection state change, and the handlers, should be executed asynchronously on the user-specified action dispatcher.
		/// Usually, this should be <c>true</c> to have a predictable behavior on the game's side.
		/// The only case where it should be <c>false</c> is when calling this from the client's destructor.
		/// </param>
		/// <returns>A task that completes when the state change and the handlers have been executed.</returns>
		STORM_NODISCARD
		pplx::task<void> setConnectionState(ConnectionState connectionState, bool async = true);

		pplx::task<std::shared_ptr<P2PScenePeer>> createScenePeerFromP2PConnection(std::shared_ptr<IConnection> connection, pplx::cancellation_token ct = pplx::cancellation_token::none());

		template<typename T1, typename T2>
		pplx::task<T1> sendSystemRequest(std::shared_ptr<IConnection> connection, byte id, const T2& parameter, pplx::cancellation_token ct = pplx::cancellation_token::none())
		{
			auto requestProcessor = dependencyResolver().resolve<RequestProcessor>();
			return requestProcessor->sendSystemRequest<T1, T2>(connection.get(), id, parameter, ct);
		}

		//initializes the scene after construction
		void initialize(DependencyScope& parentScope);
	
		void raisePeerConnected(const std::string& sessionId);

		void raisePeerDisconnected(const std::string& sessionId);

		pplx::task<void> disconnectAllPeers(pplx::cancellation_token ct = pplx::cancellation_token::none());

		pplx::task<void> disconnectFromHost(pplx::cancellation_token ct = pplx::cancellation_token::none());

		void setConnectionStateInternal(ConnectionState connectionState);

#pragma endregion

#pragma region private_members

		DependencyScope _scope;

		/// Scene host
		const bool _isHost = false;

		/// Scene peer connection.
		std::weak_ptr<IConnection> _peer;

		/// Application token.
		std::string _token;

		/// Scene metadatas.
		std::unordered_map<std::string, std::string> _metadata;

		/// Scene address.
		SceneAddress _address;

		/// The local routes.
		std::unordered_map<std::string, Route_ptr> _localRoutes;

		/// The remote routes.
		std::unordered_map<std::string, Route_ptr> _remoteRoutes;

		/// Route handlers.
		std::unordered_map<uint16, std::list<std::function<void(Packet_ptr)>>> _handlers;

		/// Owner client.
		std::weak_ptr<Client> _client;

		/// RX subscriptions for disconnection.
		std::vector<rxcpp::subscription> _subscriptions;

		/// Scene peer connection
		std::shared_ptr<IScenePeer> _host;

		/// Connection requested state (when not completed).
		bool _connecting = false;

		/// Disconnection task.
		pplx::task<void> _connectTask;

		std::recursive_mutex _mutex;

		/// Disconnection task.
		pplx::task<void> _disconnectTask;

		Event<Packet_ptr> _onPacketReceived;

		/// Scene connected state.
		ConnectionState _connectionState = ConnectionState::Disconnected;
		bool _connectionStateObservableCompleted = false;
		rxcpp::subjects::subject<ConnectionState> _sceneConnectionStateObservable;
		rxcpp::subscription _hostConnectionStateSubscription;

		rxcpp::subjects::subject<P2PConnectionStateChangedArgs> _p2pConnectionStateChangedObservable;

		/// map[sessionId] = P2PScenePeer
		std::unordered_map<std::string, std::shared_ptr<IP2PScenePeer>> _connectedPeers;
		Event<std::shared_ptr<IP2PScenePeer>> _onPeerConnected;
		Event<std::shared_ptr<IP2PScenePeer>> _onPeerDisconnected;

		std::shared_ptr<P2PService> _p2p;

		ILogger_ptr _logger;
		std::vector<IPlugin*> _plugins;

		bool _disconnectRequested = false;
		pplx::cancellation_token_source _connectionCts;

		// Store this here because we need it for Disconnected handlers execution, and at this time the DependencyScope could be invalid.
		std::shared_ptr<IActionDispatcher> _dispatcher;

		// allows the mutex to be locked in getCurrentConnectionState().
		// acceptable in this case because the mutex isn't really part of the scene's state.
		mutable std::mutex _connectionStateMutex;

#pragma endregion
	};
}

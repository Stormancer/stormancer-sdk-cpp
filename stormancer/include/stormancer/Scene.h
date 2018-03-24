#pragma once

#include "stormancer/headers.h"
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

namespace Stormancer
{
	class Client;

	/// Communicates with the server scene.
	/// Get a scene by calling Client::getPublicScene or Client::getScene.
	class Scene : public std::enable_shared_from_this<Scene>
	{
	public:

		/// The Client need to access some private members of the Scene.
		friend class Client;

		/// The SceneDispatcher need to access some private members of the Scene.
		friend class SceneDispatcher;

#pragma region public_methods

		/// Constructor.
		/// \param connection The connection to the scene.
		/// \param client The client which manage the connection.
		/// \param id Scene id.
		/// \param token Application token.
		/// \param dto Scene informations.
		Scene(IConnection* connection, std::weak_ptr<Client> client, const std::string& id, const std::string& token, const SceneInfosDto& dto, std::weak_ptr<DependencyResolver> parentDependencyResolver);

		/// Destructor.
		~Scene();

		/// initializer
		void initialize();

		/// Connect to the scene.
		STORMANCER_DLL_API pplx::task<void> connect();

		/// Disconnect from the scene.
		STORMANCER_DLL_API pplx::task<void> disconnect(bool immediate = false);

		/// Add a route to the scene.
		/// \param routeName Route name.
		/// \param handler Function which handle the receiving messages from the server on the route.
		/// \param metadata Metadatas about the Route.
		/// Add a route for each different message type.
		STORMANCER_DLL_API void addRoute(const std::string& routeName, std::function<void(Packetisp_ptr)> handler, MessageOriginFilter origin = MessageOriginFilter::Host, const std::map<std::string, std::string>& metadata = std::map<std::string, std::string>());

		/// Send a packet to a route.
		/// \param peerFilter Peer receiver.
		/// \param routeName Route name.
		/// \param writer Function where we write the data in the byte stream.
		/// \param priority Message priority on the network.
		/// \param reliability Message reliability behavior.
		STORMANCER_DLL_API void send(const PeerFilter& peerFilter, const std::string& routeName, const Writer& writer, PacketPriority priority = PacketPriority::MEDIUM_PRIORITY, PacketReliability reliability = PacketReliability::RELIABLE_ORDERED, const std::string& channelIdentifier = "");

		/// Send a packet to a route.
		/// \param routeName Route name.
		/// \param writer Function where we write the data in the byte stream.
		/// \param priority Message priority on the network.
		/// \param reliability Message reliability behavior.
		STORMANCER_DLL_API void send(const std::string& routeName, const Writer& writer, PacketPriority priority = PacketPriority::MEDIUM_PRIORITY, PacketReliability reliability = PacketReliability::RELIABLE_ORDERED, const std::string& channelIdentifier = "");

		/// Returns the connection state to the the scene.
		STORMANCER_DLL_API ConnectionState getCurrentConnectionState() const;

		STORMANCER_DLL_API rxcpp::observable<ConnectionState> getConnectionStateChangedObservable() const;

		/// Returns the scene id.
		STORMANCER_DLL_API std::string id() const;

		/// Returns the scene handle.
		STORMANCER_DLL_API byte handle() const;

		/// Returns a host metadata value.
		STORMANCER_DLL_API std::string getHostMetadata(const std::string& key) const;

		/// Returns the host connection.
		STORMANCER_DLL_API IConnection* hostConnection() const;

		/// Returns a copy of the local routes.
		STORMANCER_DLL_API std::vector<Route_ptr> localRoutes() const;

		/// Returns a copy of the remote routes.
		STORMANCER_DLL_API std::vector<Route_ptr> remoteRoutes() const;

		/// Creates an IObservable<Packet> instance that listen to events on the specified route.
		/// \param A string containing the name of the route to listen to.
		/// \return An IObservable<Packet> instance that fires each time a message is received on the route.
		STORMANCER_DLL_API rxcpp::observable<Packetisp_ptr> onMessage(const std::string& routeName, MessageOriginFilter filter = MessageOriginFilter::Host, const std::map<std::string, std::string>& metadata = std::map<std::string, std::string>());

		/// Get a vector containing the scene host connections.
		/// \return A vector containing the scene host connections.
		STORMANCER_DLL_API std::vector<IScenePeer*> remotePeers() const;

		/// Returns the peer connection to the host.
		STORMANCER_DLL_API IScenePeer* host() const;

		STORMANCER_DLL_API DependencyResolver* dependencyResolver() const;

		/// Fire when a packet is received in the scene. 
		STORMANCER_DLL_API Action<Packet_ptr>::TIterator onPacketReceived(std::function<void(Packet_ptr)> callback);

		STORMANCER_DLL_API Action<Packet_ptr>& packetReceivedAction();

		STORMANCER_DLL_API bool isHost() const;

		STORMANCER_DLL_API std::unordered_map<uint64, std::shared_ptr<IScenePeer>> connectedPeers() const;

		STORMANCER_DLL_API rxcpp::observable<P2PConnectionStateChangedArgs> p2pConnectionStateChanged() const;

		std::shared_ptr<P2PTunnel> registerP2PServer(const std::string& p2pServerId);

		pplx::task<std::shared_ptr<P2PScenePeer>> openP2PConnection(const std::string& token);

#pragma endregion

	private:

#pragma region private_methods

		/// Copy constructor deleted.
		Scene(const Scene& other) = delete;

		/// Copy operator deleted.
		Scene& operator=(const Scene& other) = delete;

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

		void setConnectionState(ConnectionState connectionState);

		pplx::task<std::shared_ptr<P2PScenePeer>> createScenePeerFromP2PConnection(std::shared_ptr<IConnection > c);

		template<typename T1, typename T2>
		pplx::task<T1> sendSystemRequest(std::shared_ptr<IConnection> connection, byte id, const T2& parameter)
		{
			
			auto requestProcessor = _dependencyResolver->resolve<RequestProcessor>();
			return requestProcessor->sendSystemRequest<T2, T1>(connection.get(), id, parameter);
		}

#pragma endregion

#pragma region private_members

		std::shared_ptr<DependencyResolver> _dependencyResolver;

		/// Scene host
		const bool _isHost = false;

		/// Scene peer connection.
		IConnection* _peer = nullptr;

		/// Application token.
		std::string _token;

		/// Scene handle.
		byte _handle = 0;

		/// Scene metadatas.
		std::map<std::string, std::string> _metadata;

		/// Scene id.
		std::string _id;

		/// The local routes.
		std::map<std::string, Route_ptr> _localRoutesMap;

		/// The remote routes.
		std::map<std::string, Route_ptr> _remoteRoutesMap;

		/// Route handlers.
		std::map<uint16, std::list<std::function<void(Packet_ptr)>>> _handlers;

		/// Owner client.
		std::weak_ptr<Client> _client;

		/// RX subscriptions for disconnection.
		std::vector<rxcpp::subscription> _subscriptions;

		/// Scene peer connection
		std::shared_ptr<IScenePeer> _host = nullptr;

		/// Connection requested state (when not completed).
		bool _connecting = false;

		/// Disconnection task.
		pplx::task<void> _connectTask;

		std::mutex _connectMutex;

		/// Disconnection task.
		pplx::task<void> _disconnectTask;

		std::mutex _disconnectMutex;

		/// Registered components
		std::map<std::string, std::function<void*()>> _registeredComponents;

		Action<Packet_ptr> _onPacketReceived;

		/// Scene connected state.
		ConnectionState _connectionState = ConnectionState::Disconnected;
		bool _connectionStateObservableCompleted = false;
		rxcpp::subjects::subject<ConnectionState> _connectionStateObservable;

		rxcpp::subjects::subject<P2PConnectionStateChangedArgs> _p2pConnectionStateChangedObservable;
		std::unordered_map<uint64, std::shared_ptr<IScenePeer>> _connectedPeers;

		std::shared_ptr<P2PService> _p2p;

		ILogger_ptr _logger;

#pragma endregion
	};

	using Scene_ptr = std::shared_ptr<Scene>;
};

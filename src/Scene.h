#pragma once
#include "headers.h"
#include "ConnectionResult.h"
#include "SceneInfosDto.h"
#include "Route.h"
#include "IScenePeer.h"
#include "Packet.h"
#include "IObservable.h"

namespace Stormancer
{
	class Client;

	/// Communicates with the server scene.
	/// Get a scene by calling Client::getPublicScene or Client::getScene.
	class Scene
	{
		/// The Client need to access some private members of the Scene.
		friend class Client;

		/// The SceneDispatcher need to access some private members of the Scene.
		friend class SceneDispatcher;

	public:

		/// Constructor.
		/// \param connection The connection to the scene.
		/// \param client The client which manage the connection.
		/// \param id Scene id.
		/// \param token Application token.
		/// \param dto Scene informations.
		Scene(IConnection* connection, Client* client, std::string id, std::string token, SceneInfosDto dto, DependencyResolver* parentDependencyResolver, std::function<void()> onDelete);

		/// Destructor.
		virtual ~Scene();

		/// Copy constructor deleted.
		Scene(const Scene& other) = delete;

		/// Copy operator deleted.
		Scene& operator=(const Scene& other) = delete;

	public:

		/// Connect to the scene.
		STORMANCER_DLL_API pplx::task<Result<>*> connect();

		/// Disconnect from the scene.
		STORMANCER_DLL_API pplx::task<Result<>*> disconnect(bool immediate = false);

		/// Add a route to the scene.
		/// \param routeName Route name.
		/// \param handler Function which handle the receiving messages from the server on the route.
		/// \param metadata Metadatas about the Route.
		/// Add a route for each different message type.
		STORMANCER_DLL_API Result<>* addRoute(const char* routeName, std::function<void(Packetisp_ptr)> handler, const stringMap* metadata = nullptr);

		/// Send a packet to a route.
		/// \param routeName Route name.
		/// \param writer Function where we write the data in the byte stream.
		/// \param priority Message priority on the network.
		/// \param reliability Message reliability behavior.
		STORMANCER_DLL_API Result<>* sendPacket(const char* routeName, std::function<void(bytestream*)> writer, PacketPriority priority = PacketPriority::MEDIUM_PRIORITY, PacketReliability reliability = PacketReliability::RELIABLE);

		/// Returns the connection state to the the scene.
		STORMANCER_DLL_API ConnectionState connectionState();

		STORMANCER_DLL_API Action<ConnectionState>& onConnectionStateChangedAction();

		STORMANCER_DLL_API Action<ConnectionState>::TIterator onConnectionStateChanged(std::function<void(ConnectionState)> callback);

		/// Returns the scene id.
		STORMANCER_DLL_API const char* id();

		/// Returns the scene handle.
		STORMANCER_DLL_API byte handle();

		/// Returns a host metadata value.
		STORMANCER_DLL_API const char* getHostMetadata(const char* key);

		/// Returns the host connection.
		STORMANCER_DLL_API IConnection* hostConnection();

		/// Returns a copy of the local routes.
		STORMANCER_DLL_API DataStructures::List<Route_ptr> localRoutes();

		/// Returns a copy of the remote routes.
		STORMANCER_DLL_API DataStructures::List<Route_ptr> remoteRoutes();

		/// Creates an IObservable<Packet> instance that listen to events on the specified route.
		/// \param route A pointer of the Route instance to listen to.
		/// \return An IObservable<Packet> instance that fires each time a message is received on the route.
		STORMANCER_DLL_API IObservable<Packetisp_ptr>* onMessage(Route_ptr);

		/// Creates an IObservable<Packet> instance that listen to events on the specified route.
		/// \param A string containing the name of the route to listen to.
		/// \return An IObservable<Packet> instance that fires each time a message is received on the route.
		STORMANCER_DLL_API IObservable<Packetisp_ptr>* onMessage(const char* routeName);

		/// Get a vector containing the scene host connections.
		/// \return A vector containing the scene host connections.
		STORMANCER_DLL_API DataStructures::List<IScenePeer*> remotePeers();

		/// Returns the peer connection to the host.
		STORMANCER_DLL_API IScenePeer* host();

		STORMANCER_DLL_API DependencyResolver* dependencyResolver() const;

		/// Fire when a packet is received in the scene. 
		STORMANCER_DLL_API Action<Packet_ptr>::TIterator onPacketReceived(std::function<void(Packet_ptr)> callback);

		STORMANCER_DLL_API Action<Packet_ptr>& packetReceivedAction();

		/// Finalize the connection to the scene.
		/// \param cr Connection result message retrieved by a system request.
		void completeConnectionInitialization(ConnectionResult& cr);

		/// Handle a message received on the scene and dispatch the packet to the right route handle.
		/// \param packet Receivedpacket.
		void handleMessage(Packet_ptr packet);

		bool isHost() const;

	private:
		void setConnectionState(ConnectionState connectionState);

	private:

		const bool _isHost = false;

		/// Scene peer connection.
		IConnection* _peer;

		/// Application token.
		std::string _token;

		/// Scene handle.
		byte _handle;

		/// Scene metadatas.
		stringMap _metadata;

		/// Scene id.
		std::string _id;

		/// The local routes.
		std::map<std::string, Route_ptr> _localRoutesMap;

		/// The remote routes.
		std::map<std::string, Route_ptr> _remoteRoutesMap;

		/// Route handlers.
		std::map<uint16, std::list<std::function<void(Packet_ptr)>>> _handlers;

		/// Owner client.
		Client* _client;

		/// RX subscriptions for disconnection.
		std::vector<std::weak_ptr<ISubscription>> _subscriptions;

		/// Scene peer connection
		IScenePeer* _host = nullptr;

		/// Connection requested state (when not completed).
		bool _connecting = false;

		/// Disconnection task.
		pplx::task<void> _connectTask;

		/// Disconnection task.
		pplx::task<void> _disconnectTask;

		/// Registered components
		std::map<std::string, std::function<void*()>> _registeredComponents;

		/// Event launched on instance deletion
		std::function<void()> _onDelete;

		DependencyResolver* _dependencyResolver = nullptr;

		Action<Packet_ptr> _onPacketReceived;

		/// Scene connected state.
		ConnectionState _connectionState = ConnectionState::Disconnected;
		Action<ConnectionState> _onConnectionStateChanged;
		Action<ConnectionState>::TIterator _peerConnectionStateEraseIterator;
	};
};

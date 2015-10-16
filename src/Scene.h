#pragma once
#include "headers.h"
#include "ConnectionResult.h"
#include "SceneInfosDto.h"
#include "Route.h"
#include "IScenePeer.h"
#include "Packet.h"

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
		Scene(IConnection* connection, Client* client, std::string id, std::string token, SceneInfosDto dto, Action<void> onDelete);

		/// Destructor.
		virtual ~Scene();

		/// Copy constructor deleted.
		Scene(Scene& other) = delete;

		/// Copy operator deleted.
		Scene& operator=(Scene& other) = delete;

	public:

		/// Connect to the scene.
		STORMANCER_DLL_API pplx::task<void> connect();

		/// Disconnect from the scene.
		STORMANCER_DLL_API pplx::task<void> disconnect();

		/// Add a route to the scene.
		/// \param routeName Route name.
		/// \param handler Function which handle the receiving messages from the server on the route.
		/// \param metadata Metadatas about the Route.
		/// Add a route for each different message type.
		STORMANCER_DLL_API void addRoute(std::string routeName, std::function<void(Packetisp_ptr)> handler, stringMap metadata = stringMap());

		/// Send a packet to a route.
		/// \param routeName Route name.
		/// \param writer Function where we write the data in the byte stream.
		/// \param priority Message priority on the network.
		/// \param reliability Message reliability behavior.
		STORMANCER_DLL_API void sendPacket(std::string routeName, std::function<void(bytestream*)> writer, PacketPriority priority = PacketPriority::MEDIUM_PRIORITY, PacketReliability reliability = PacketReliability::RELIABLE);

		/// Returns the connected state to the the scene.
		STORMANCER_DLL_API bool connected();

		/// Returns the scene id.
		STORMANCER_DLL_API std::string id();

		/// Returns the scene handle.
		STORMANCER_DLL_API byte handle();

		/// Returns a host metadata value.
		STORMANCER_DLL_API std::string getHostMetadata(std::string key);

		/// Returns the host connection.
		STORMANCER_DLL_API IConnection* hostConnection();

		/// Returns a copy of the local routes.
		STORMANCER_DLL_API std::vector<Route_ptr> localRoutes();

		/// Returns a copy of the remote routes.
		STORMANCER_DLL_API std::vector<Route_ptr> remoteRoutes();

		/// Creates an IObservable<Packet> instance that listen to events on the specified route.
		/// \param route A pointer of the Route instance to listen to.
		/// \return An IObservable<Packet> instance that fires each time a message is received on the route.
		STORMANCER_DLL_API rxcpp::observable<Packetisp_ptr> onMessage(Route_ptr);

		/// Creates an IObservable<Packet> instance that listen to events on the specified route.
		/// \param A string containing the name of the route to listen to.
		/// \return An IObservable<Packet> instance that fires each time a message is received on the route.
		STORMANCER_DLL_API rxcpp::observable<Packetisp_ptr> onMessage(std::string routeName);

		/// Get a vector containing the scene host connections.
		/// \return A vector containing the scene host connections.
		STORMANCER_DLL_API std::vector<IScenePeer*> remotePeers();

		/// Returns the peer connection to the host.
		STORMANCER_DLL_API IScenePeer* host();

		STORMANCER_DLL_API void registerComponent(std::string componentName, std::function<void*()> factory);

		STORMANCER_DLL_API void* getComponent(std::string componentName);

		/// Finalize the connection to the scene.
		/// \param cr Connection result message retrieved by a system request.
		void completeConnectionInitialization(ConnectionResult& cr);

		/// Handle a message received on the scene and dispatch the packet to the right route handle.
		/// \param packet Receivedpacket.
		void handleMessage(Packet_ptr packet);

	public:

		/// Fire when a packet is received in the scene. 
		Action<Packet_ptr> packetReceived;

		const bool isHost = false;

	private:

		/// Scene connected state.
		bool _connected = false;

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
		std::vector<rxcpp::composite_subscription> subscriptions;

		/// Scene peer connection
		IScenePeer* _host = nullptr;

		/// Connection requested state (when not completed).
		bool connectCalled = false;

		/// Connection task.
		pplx::task<void> connectTask;

		/// Disconnection request state (when not completed).
		bool disconnectCalled = false;

		/// Disconnection task.
		pplx::task<void> disconnectTask;

		/// Registered components
		std::map<std::string, std::function<void*()>> _registeredComponents;

		/// Event launched on instance deletion
		Action<void> _onDelete;
	};
};

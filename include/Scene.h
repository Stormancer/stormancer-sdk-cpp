#pragma once
#include "headers.h"
#include "Dto/ConnectionResult.h"
#include "Dto/SceneInfosDto.h"
#include "Route.h"
#include "IScenePeer.h"
#include "Packet.h"

namespace Stormancer
{
	using PacketObservable = rx::observable < shared_ptr<Packet<>> > ;

	class Client;

	/*! The scene class for communicating with the server scene.
	*/
	class Scene
	{
		friend class Client;
		friend class SceneDispatcher;

	public:
		Scene(IConnection* connection, Client* client, wstring id, wstring token, SceneInfosDto dto);
		virtual ~Scene();

		Scene(Scene& other) = delete;
		Scene& operator=(Scene& other) = delete;

	public:

		/*! Connect to the scene.
		*/
		STORMANCER_DLL_API pplx::task<void> connect();

		/*! Disconnect from the scene.
		*/
		STORMANCER_DLL_API pplx::task<void> disconnect();

		/*! Add a route to the scene.
		\param routeName The name of the route.
		\param handler Message function which handle the receiving messages from the server on the route.
		\param metadata Some metadata on the Route.
		Add a route for each different message type.
		*/
		STORMANCER_DLL_API void addRoute(wstring routeName, function<void(shared_ptr<Packet<IScenePeer>>)> handler, stringMap metadata = stringMap());
		
		/*! Send a packet to a route.
		\param routeName The name of the route.
		\param writer The writer lambda where we write the message in the byte stream.
		\param priority The message priority on the network.
		\param reliability The message reliability behavior.
		*/
		STORMANCER_DLL_API void sendPacket(wstring routeName, function<void(bytestream*)> writer, PacketPriority priority = PacketPriority::MEDIUM_PRIORITY, PacketReliability reliability = PacketReliability::RELIABLE);
		
		/*! Get the connected state to the the scene.
		\return The connected state to the server.
		*/
		STORMANCER_DLL_API bool connected();
		
		/*! Get the scene id.
		\return The scene id.
		*/
		STORMANCER_DLL_API wstring id();
		
		/*! Get the scene handle.
		\return The scene handle.
		*/
		STORMANCER_DLL_API byte handle();
		
		/*! Get a host metadata value.
		\param key The key.
		\return The value.
		*/
		STORMANCER_DLL_API wstring getHostMetadata(wstring key);
		
		/*! Get the host connection.
		\return The host connection.
		*/
		STORMANCER_DLL_API IConnection* hostConnection();
		
		/*! Get the local routes in a vector.
		\return A vector containing the local routes.
		*/
		STORMANCER_DLL_API vector<Route*> localRoutes();
		
		/*! Get the remote routes.
		\return A vector containing the remote routes.
		*/
		STORMANCER_DLL_API vector<Route*> remoteRoutes();
		
		/*! Creates an IObservable<Packet> instance that listen to events on the specified route.
		\param route A pointer of the Route instance to listen to.
		\return An IObservable<Packet> instance that fires each time a message is received on the route.
		*/
		STORMANCER_DLL_API rx::observable<shared_ptr<Packet<IScenePeer>>> onMessage(Route* route);

		/*! Creates an IObservable<Packet> instance that listen to events on the specified route.
		\param A string containing the name of the route to listen to.
		\return An IObservable<Packet> instance that fires each time a message is received on the route.
		*/
		STORMANCER_DLL_API rx::observable<shared_ptr<Packet<IScenePeer>>> onMessage(wstring routeName);

		/*! Get a vector containing the scene host connections.
		\return A vector containing the scene host connections.
		*/
		STORMANCER_DLL_API vector<IScenePeer*> remotePeers();
		
		/*! Get the peer connection to the host.
		\return The peer connection to the host.
		*/
		STORMANCER_DLL_API IScenePeer* host();
		
		void completeConnectionInitialization(ConnectionResult& cr);
		void handleMessage(shared_ptr<Packet<>> packet);

	public:
		Action<shared_ptr<Packet<>>> packetReceived;
		const bool isHost = false;

	private:
		bool _connected = false;
		IConnection* _peer;
		wstring _token;
		byte _handle;
		stringMap _metadata;
		wstring _id;
		map<wstring, Route> _localRoutesMap;
		map<wstring, Route> _remoteRoutesMap;
		map<uint16, vector<function<void(shared_ptr<Packet<>>)>*>> _handlers;
		Client* _client;
		vector<rxcpp::composite_subscription> subscriptions;
		IScenePeer* _host = nullptr;
		bool connectCalled = false;
		pplx::task<void> connectTask;
		bool disconnectCalled = false;
		pplx::task<void> disconnectTask;
	};
};

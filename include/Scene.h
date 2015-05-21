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
		STORMANCER_DLL_API pplx::task<void> connect();
		STORMANCER_DLL_API pplx::task<void> disconnect();
		STORMANCER_DLL_API void addRoute(wstring routeName, function<void(shared_ptr<Packet<IScenePeer>>)> handler, stringMap metadata = stringMap());
		STORMANCER_DLL_API void sendPacket(wstring routeName, function<void(bytestream*)> writer, PacketPriority priority = PacketPriority::MEDIUM_PRIORITY, PacketReliability reliability = PacketReliability::RELIABLE);
		STORMANCER_DLL_API bool connected();
		STORMANCER_DLL_API wstring id();
		STORMANCER_DLL_API byte handle();
		STORMANCER_DLL_API wstring getHostMetadata(wstring key);
		STORMANCER_DLL_API IConnection* hostConnection();
		STORMANCER_DLL_API vector<Route*> localRoutes();
		STORMANCER_DLL_API vector<Route*> remoteRoutes();
		STORMANCER_DLL_API rx::observable<shared_ptr<Packet<IScenePeer>>> onMessage(Route* route);
		STORMANCER_DLL_API rx::observable<shared_ptr<Packet<IScenePeer>>> onMessage(wstring routeName);
		STORMANCER_DLL_API vector<IScenePeer*> remotePeers();
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

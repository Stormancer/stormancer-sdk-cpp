#pragma once
#include "headers.h"
#include "Dto/ConnectionResult.h"
#include "Dto/SceneInfosDto.h"
#include "Route.h"
#include "IScenePeer.h"
#include "Packet.h"

namespace Stormancer
{
	using PacketObservable = rx::observable < Packet<>* > ;

	class Client;

	class Scene
	{
		friend class Client;
		friend class SceneDispatcher;

	private:
		Scene(IConnection* connection, Client* client, wstring id, wstring token, SceneInfosDto dto);
		virtual ~Scene();

	public:
		STORMANCER_DLL_API void addRoute(wstring routeName, function<void(Packet<IScenePeer>*)> handler, stringMap metadata = stringMap());
		STORMANCER_DLL_API void sendPacket(wstring routeName, function<void(bytestream*)> writer, PacketPriority priority = PacketPriority::MEDIUM_PRIORITY, PacketReliability reliability = PacketReliability::RELIABLE);
		STORMANCER_DLL_API pplx::task<void> connect();
		wstring getHostMetadata(wstring key);
		byte handle();
		wstring id();
		bool connected();
		IConnection* hostConnection();
		vector<Route*> localRoutes();
		vector<Route*> remoteRoutes();
		rx::observable<Packet<IScenePeer>*> onMessage(Route* route);
		rx::observable<Packet<IScenePeer>*> onMessage(wstring routeName);
		pplx::task<void> disconnect();
		vector<IScenePeer*> remotePeers();
		IScenePeer* host();

	private:
		void completeConnectionInitialization(ConnectionResult& cr);
		void handleMessage(Packet<>* packet);

	public:
		vector<function<void(Packet<>*)>> packetReceived;
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
		map<uint16, vector<function<void(Packet<>*)>*>> _handlers;
		Client* _client;
	};
};

#pragma once
#include "headers.h"
#include "Core/Route.h"
#include "SystemMessages/SceneInfosDto.h"
#include "SystemMessages/ConnectionResult.h"
#include "Core/IScenePeer.h"

namespace Stormancer
{
	class Client;

	class Scene
	{
		friend class Client;
		friend class SceneDispatcher;

	private:
		Scene(IConnection* connection, Client* client, wstring id, wstring token, SceneInfosDto dto);
		virtual ~Scene();

	public:
		wstring getHostMetadata(wstring key);
		byte handle();
		wstring id();
		bool connected();
		IConnection* hostConnection();
		vector<Route*> localRoutes();
		vector<Route*> remoteRoutes();
		void addRoute(wstring routeName, function<void(Packet<IScenePeer>*)> handler, stringMap metadata = stringMap());
		rx::observable<Packet<IScenePeer>*>* onMessage(Route* route);
		rx::observable<Packet<IScenePeer>*>* onMessage(wstring routeName);
		void sendPacket(wstring routeName, function<void(byteStream*)> writer, PacketPriority priority = PacketPriority::MEDIUM_PRIORITY, PacketReliability reliability = PacketReliability::RELIABLE);
		pplx::task<void> connect();
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
		IConnection* _peer;
		wstring _token;
		byte _handle;
		stringMap _metadata;
		wstring _id;
		bool _connected;
		map<wstring, Route> _localRoutesMap;
		map<wstring, Route> _remoteRoutesMap;
		map<uint16, vector<function<void(Packet<>*)>>> _handlers;
		Client* _client;
	};
};

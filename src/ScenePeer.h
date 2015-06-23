#pragma once
#include "headers.h"
#include "IScenePeer.h"
#include "Scene.h"
#include "Route.h"

namespace Stormancer
{
	/// Remote scene connection.
	class ScenePeer : public IScenePeer
	{
	public:
		ScenePeer(IConnection* connection, byte sceneHandle, map<wstring, Route>& routeMapping, Scene* scene);
		virtual ~ScenePeer();

	public:
		void send(wstring& routeName, function<void(bytestream*)> writer, PacketPriority priority, PacketReliability reliability);
		void disconnect();
		uint64 id();

	private:
		IConnection* _connection;
		byte _sceneHandle;
		map<wstring, Route> _routeMapping;
		Scene* _scene;
	};
};
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
		ScenePeer(IConnection* connection, byte sceneHandle, std::map<std::string, Route>& routeMapping, Scene* scene);
		virtual ~ScenePeer();

	public:
		void send(std::string& routeName, std::function<void(bytestream*)> writer, PacketPriority priority, PacketReliability reliability);
		void disconnect();
		uint64 id();

	private:
		IConnection* _connection;
		byte _sceneHandle;
		std::map<std::string, Route> _routeMapping;
		Scene* _scene;
	};
};

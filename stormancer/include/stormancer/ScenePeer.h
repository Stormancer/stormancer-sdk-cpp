#pragma once

#include "stormancer/headers.h"
#include "stormancer/IScenePeer.h"
#include "stormancer/Scene.h"
#include "stormancer/Route.h"
#include "stormancer/PacketPriority.h"

namespace Stormancer
{
	/// Remote scene connection.
	class ScenePeer : public IScenePeer
	{
	public:

#pragma region public_methods

		ScenePeer(IConnection* connection, byte sceneHandle, std::map<std::string, Route_ptr>& routeMapping, Scene* scene);
		~ScenePeer();
		void send(const std::string& routeName, const Writer& writer, PacketPriority priority = PacketPriority::MEDIUM_PRIORITY, PacketReliability reliability = PacketReliability::RELIABLE_ORDERED) override;
		void disconnect() override;
		uint64 id() override;

#pragma endregion

	private:

#pragma region private_members

		IConnection* _connection;
		byte _sceneHandle;
		std::map<std::string, Route_ptr> _routeMapping;
		Scene* _scene;

#pragma endregion
	};
};

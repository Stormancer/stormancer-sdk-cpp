#pragma once

#include "stormancer/BuildConfig.h"


#include "stormancer/IScenePeer.h"
#include "stormancer/SceneImpl.h"
#include "stormancer/Route.h"
#include "stormancer/PacketPriority.h"

namespace Stormancer
{
	/// Remote scene connection.
	class ScenePeer : public IScenePeer
	{
	public:

#pragma region public_methods

		ScenePeer(std::weak_ptr<IConnection> connection, byte sceneHandle, std::map<std::string, Route_ptr>& routeMapping, std::weak_ptr<Scene> scene);
		~ScenePeer();
		void send(const std::string& routeName, const StreamWriter& streamWriter, PacketPriority priority = PacketPriority::MEDIUM_PRIORITY, PacketReliability reliability = PacketReliability::RELIABLE_ORDERED) override;
		void disconnect() override;
		uint64 id() override;

		std::string getSceneId() const override;
#pragma endregion

	private:

#pragma region private_members

		std::weak_ptr<IConnection> _connection;
		byte _sceneHandle;
		std::map<std::string, Route_ptr> _routeMapping;
		std::weak_ptr<Scene> _scene;

#pragma endregion
	};
};

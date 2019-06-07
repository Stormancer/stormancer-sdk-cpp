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

		ScenePeer(std::weak_ptr<IConnection> connection, byte sceneHandle, std::unordered_map<std::string, Route_ptr>& routeMapping, std::weak_ptr<Scene> scene);
		
		uint64 id() const override;

		std::string getSceneId() const override;

		std::shared_ptr<IConnection> connection() const override;

		const std::unordered_map<std::string, Route_ptr>& routes() const override;

		void send(const std::string& routeName, const StreamWriter& streamWriter, PacketPriority priority = PacketPriority::MEDIUM_PRIORITY, PacketReliability reliability = PacketReliability::RELIABLE_ORDERED, const std::string& channelIdentifier = "") override;

		void disconnect() override;

#pragma endregion

	private:

#pragma region private_members

		byte _sceneHandle;

		std::unordered_map<std::string, Route_ptr> _routes;

		std::shared_ptr<IConnection> _connection;

		std::weak_ptr<Scene> _scene;

#pragma endregion
	};
}

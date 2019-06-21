#pragma once

#include "stormancer/BuildConfig.h"

#include "stormancer/P2P/IP2PScenePeer.h"
#include "stormancer/P2P/P2PService.h"

namespace Stormancer
{
	class Scene;

	class P2PScenePeer : public IP2PScenePeer
	{
	public:

#pragma region public_methods

		P2PScenePeer(std::weak_ptr<Scene> scene, std::shared_ptr<IConnection> connection, std::shared_ptr<P2PService> P2pService, P2PConnectToSceneMessage message);

		uint64 id() const override;

		std::string getSceneId() const override;

		std::shared_ptr<IConnection> connection() const override;

		const std::unordered_map<std::string, Route_ptr>& routes() const override;

		void send(const std::string& route, const StreamWriter& streamWriter, PacketPriority priority = PacketPriority::MEDIUM_PRIORITY, PacketReliability reliability = PacketReliability::RELIABLE_ORDERED, const std::string& channelIdentifier = "") override;

		void disconnect() override;

		pplx::task<std::shared_ptr<P2PTunnel>> openP2PTunnel(const std::string& serverId, pplx::cancellation_token ct = pplx::cancellation_token::none()) override;

		byte handle() const override;

		std::string sessionId() const override;

#pragma endregion

	private:

#pragma region private_members

		/// Scene handle.
		byte _handle = 0;

		std::unordered_map<std::string, std::string> _metadata;

		/// The remote routes.
		std::unordered_map<std::string, Route_ptr> _routes;

		std::shared_ptr<IConnection> _connection;

		std::weak_ptr<Scene> _scene;

		std::shared_ptr<P2PService> _p2pService;

#pragma endregion
	};
}

#pragma once

#include "stormancer/BuildConfig.h"


#include "stormancer/P2P/IP2PScenePeer.h"
#include "stormancer/P2P/P2PService.h"


namespace Stormancer
{
	class Scene;
	struct P2PConnectToSceneMessage;
	class P2PScenePeer : public IP2PScenePeer
	{
	public:

#pragma region public_methods

		P2PScenePeer(std::weak_ptr<Scene> scene, std::shared_ptr<IConnection> connection, std::shared_ptr<P2PService> P2P, P2PConnectToSceneMessage);

		std::string getSceneId() const override;

		void send(const std::string& route, const StreamWriter& streamWriter, PacketPriority packetPriority = PacketPriority::MEDIUM_PRIORITY, PacketReliability packetReliability = PacketReliability::RELIABLE_ORDERED) override;

		uint64 id() override;
		void disconnect() override;
		pplx::task<std::shared_ptr<P2PTunnel>> openP2PTunnel(const std::string& serverId, pplx::cancellation_token ct = pplx::cancellation_token::none()) override;

#pragma endregion

	private:

#pragma region private_members

		std::weak_ptr<Scene> _scene;
		std::shared_ptr<IConnection> _connection;
		std::shared_ptr<P2PService> _p2p;

		/// Scene handle.
		byte _handle = 0;
		std::map<std::string, std::string> _metadata;
		/// The remote routes.
		std::map<std::string, Route_ptr> _remoteRoutesMap;
#pragma endregion
	};
}

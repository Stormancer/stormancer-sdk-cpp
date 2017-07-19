#pragma once

#include "headers.h"
#include "IScenePeer.h"
#include "P2P/P2PService.h"
#include "P2P/P2PTunnel.h"

namespace Stormancer
{
	class Scene;

	class P2PScenePeer : public IScenePeer
	{
	public:

#pragma region public_methods

		P2PScenePeer(Scene* scene, std::shared_ptr<IConnection> connection, std::shared_ptr<P2PService> P2P);

		std::string getSceneId() const;

		virtual void send(std::string& route, std::function<void(bytestream*)> writer, PacketPriority packetPriority = PacketPriority::MEDIUM_PRIORITY, PacketReliability packetReliability = PacketReliability::RELIABLE_ORDERED) override;

		virtual uint64 id() override;
		virtual void disconnect() override;
		pplx::task<std::shared_ptr<P2PTunnel>> openP2PTunnel(const std::string& serverId);

#pragma endregion

	private:

#pragma region private_members

		Scene* _scene;
		std::shared_ptr<IConnection> _connection;
		std::shared_ptr<P2PService> _p2p;

#pragma endregion
	};
}

#pragma once

#include "stormancer/BuildConfig.h"

#include <memory>

namespace Stormancer
{
	class IClient;
	class Scene;
	class ITransport;
	class IConnection;

	template<typename T>
	class Packet;

	class IPlugin
	{
	public:

#pragma region public_methods

		virtual ~IPlugin();
		virtual void clientCreated(std::shared_ptr<IClient> client);
		virtual void transportStarted(std::shared_ptr<ITransport> transport);
		virtual void registerSceneDependencies(std::shared_ptr<Scene> scene);
		virtual void sceneCreated(std::shared_ptr<Scene> scene);
		virtual void sceneConnecting(std::shared_ptr<Scene> scene);
		virtual void sceneConnected(std::shared_ptr<Scene> scene);
		virtual void sceneDisconnecting(std::shared_ptr<Scene> scene);
		virtual void sceneDisconnected(std::shared_ptr<Scene> scene);
		virtual void packetReceived(std::shared_ptr<Packet<IConnection>> packet);

		virtual void clientDisconnecting(std::shared_ptr<IClient> client);

#pragma endregion
	};
};

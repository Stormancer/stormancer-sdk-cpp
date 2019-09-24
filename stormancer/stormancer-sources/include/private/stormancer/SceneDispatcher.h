#pragma once

#include "stormancer/BuildConfig.h"


#include "stormancer/IPacketProcessor.h"
#include "stormancer/IActionDispatcher.h"
#include "stormancer/SceneImpl.h"

namespace Stormancer
{
	/// Dispatch the network messages to the right scene.
	class SceneDispatcher : public IPacketProcessor
	{
		friend class Client;

	public:

#pragma region public_methods

		SceneDispatcher(std::shared_ptr<IActionDispatcher> evDispatcher, std::shared_ptr<ILogger> logger);
		virtual ~SceneDispatcher() = default;

		void registerProcessor(PacketProcessorConfig& config);
		void addScene(std::shared_ptr<IConnection> connection, std::shared_ptr<Scene_Impl> scene);
		void removeScene(std::shared_ptr<IConnection> connection, uint8 sceneHandle);
		std::shared_ptr<Scene_Impl> getScene(std::shared_ptr<IConnection> connection,  uint8 sceneHandle);

#pragma endregion

	private:

#pragma region private_methods

		bool handler_impl(uint8 sceneHandle, Packet_ptr packet);
		void resize(std::vector<std::weak_ptr<Scene_Impl>>&, uint8 sceneHandle);
		uint8 getSceneIndex(uint8 sceneHandle);
		std::shared_ptr<std::vector<std::weak_ptr<Scene_Impl>>> getHandles(const IConnection& connection);

#pragma endregion

#pragma region private_members

		std::shared_ptr<IActionDispatcher> _eventDispatcher;
		std::shared_ptr<ILogger> logger;
		ProcessorFunction handler = nullptr;

#pragma endregion
	};
}

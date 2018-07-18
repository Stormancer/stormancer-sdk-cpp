#pragma once

#include "stormancer/headers.h"
#include "stormancer/IPacketProcessor.h"
#include "stormancer/IActionDispatcher.h"
#include "stormancer/Scene.h"

namespace Stormancer
{
	/// Dispatch the network messages to the right scene.
	class SceneDispatcher : public IPacketProcessor
	{
		friend class Client;

	public:

#pragma region public_methods

		SceneDispatcher(std::shared_ptr<IActionDispatcher> evDispatcher);
		virtual ~SceneDispatcher();

		void registerProcessor(PacketProcessorConfig& config);
		void addScene(Scene_ptr scene);
		void removeScene(uint8 sceneHandle);

#pragma endregion

	private:

#pragma region private_methods

		bool handler_impl(uint8 sceneHandle, Packet_ptr packet);

#pragma endregion

#pragma region private_members

		std::shared_ptr<IActionDispatcher> _eventDispatcher;
		processorFunction* handler = nullptr;
		std::vector<std::weak_ptr<Scene>> _scenes;

#pragma endregion
	};
};

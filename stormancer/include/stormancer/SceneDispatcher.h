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
		SceneDispatcher(std::shared_ptr<IActionDispatcher> evDispatcher);
		virtual ~SceneDispatcher();

	public:
		void registerProcessor(PacketProcessorConfig& config);
		void addScene(Scene_ptr scene);
		void removeScene(uint8 sceneHandle);

	private:
		std::shared_ptr<IActionDispatcher> _eventDispatcher;
		bool handler_impl(uint8 sceneHandle, Packet_ptr packet);

	private:
		processorFunction* handler = nullptr;
		std::vector<std::weak_ptr<Scene>> _scenes;
	};
};

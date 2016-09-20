#pragma once
#include "headers.h"
#include "IPacketProcessor.h"

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
		void addScene(Scene* scene);
		void removeScene(uint8 sceneHandle);

	private:
		std::shared_ptr<IActionDispatcher> _eventDispatcher;
		bool handler_impl(uint8 sceneHandle, Packet_ptr packet);

	private:
		processorFunction* handler = nullptr;
		std::vector<Scene*> _scenes;
	};
};

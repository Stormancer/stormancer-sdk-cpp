#pragma once
#include "headers.h"
#include "IPacketProcessor.h"

namespace Stormancer
{
	class SceneDispatcher : public IPacketProcessor
	{
	public:
		SceneDispatcher();
		virtual ~SceneDispatcher();

	public:
		void registerProcessor(PacketProcessorConfig& config);
		void addScene(Scene* scene);
		void removeScene(uint8 sceneHandle);

	private:
		bool handler_impl(uint8 sceneHandle, Packet<>* packet);

	private:
		processorFunction* handler = nullptr;
		vector<Scene*> _scenes;
	};
};

#include "stormancer.h"

namespace Stormancer
{
	SceneDispatcher::SceneDispatcher()
		: _scenes((uint32)MAXBYTE - (uint32)MessageIDTypes::ID_SCENES + 1, nullptr)
	{
		handler = new processorFunction([this](uint8 sceneHandle, shared_ptr<Packet<>> packet) {
			return this->handler_impl(sceneHandle, packet);
		});
	}

	SceneDispatcher::~SceneDispatcher()
	{
	}

	void SceneDispatcher::registerProcessor(PacketProcessorConfig& config)
	{
		config.addCatchAllProcessor(handler);
	}

	void SceneDispatcher::addScene(Scene* scene)
	{
		uint8 index = scene->handle() - (uint8)MessageIDTypes::ID_SCENES;
		_scenes[index] = scene;
	}

	void SceneDispatcher::removeScene(uint8 sceneHandle)
	{
		size_t index = sceneHandle - (uint8)MessageIDTypes::ID_SCENES;
		_scenes[index] = nullptr;
	}

	bool SceneDispatcher::handler_impl(uint8 sceneHandle, shared_ptr<Packet<>> packet)
	{
		if (sceneHandle < (uint8)MessageIDTypes::ID_SCENES)
		{
			return false;
		}

		size_t sceneIndex = sceneHandle - (uint8)MessageIDTypes::ID_SCENES;
		if (sceneIndex >= _scenes.size())
		{
			return false;
		}
		else
		{
			auto scene = _scenes[sceneIndex];
			packet->setMetadata(L"scene", scene);
			scene->handleMessage(packet);
			return true;
		}
	}
};

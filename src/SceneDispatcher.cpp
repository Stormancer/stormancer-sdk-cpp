#include "stormancer.h"

namespace Stormancer
{
	SceneDispatcher::SceneDispatcher()
	{
		handler = new processorFunction([this](byte sceneHandle, Packet<>* packet) {
			return this->handler_impl(sceneHandle, packet);
		});
	}

	SceneDispatcher::~SceneDispatcher()
	{
	}

	void SceneDispatcher::registerProcessor(PacketProcessorConfig* config)
	{
		config->addCatchAllProcessor(handler);
	}

	void SceneDispatcher::addScene(Scene* scene)
	{
		uint8 index = scene->handle() - (byte)MessageIDTypes::ID_SCENES;
		_scenes[index] = scene;
	}

	void SceneDispatcher::removeScene(byte sceneHandle)
	{
		size_t index = sceneHandle - (byte)MessageIDTypes::ID_SCENES;
		if (index < _scenes.size())
		{
			_scenes.erase(_scenes.begin() + index);
		}
	}

	bool SceneDispatcher::handler_impl(byte sceneHandle, Packet<>* packet)
	{
		if (sceneHandle < (byte)MessageIDTypes::ID_SCENES)
		{
			return false;
		}

		size_t sceneIndex = sceneHandle - (byte)MessageIDTypes::ID_SCENES;
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

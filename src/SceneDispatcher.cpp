#include "stormancer.h"

namespace Stormancer
{
	SceneDispatcher::SceneDispatcher()
		: _scenes((uint32)0xff - (uint32)MessageIDTypes::ID_SCENES + 1, Scene_wptr())
	{
		handler = new processorFunction([this](uint8 sceneHandle, Packet_ptr packet) {
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

	void SceneDispatcher::addScene(Scene_wptr scene_wptr)
	{
		auto scene = scene_wptr.lock();
		if (scene)
		{
			uint8 index = scene->handle() - (uint8)MessageIDTypes::ID_SCENES;
			_scenes[index] = scene;
		}
	}

	void SceneDispatcher::removeScene(uint8 sceneHandle)
	{
		size_t index = sceneHandle - (uint8)MessageIDTypes::ID_SCENES;
		_scenes[index].reset();
	}

	bool SceneDispatcher::handler_impl(uint8 sceneHandle, Packet_ptr packet)
	{
		if (sceneHandle < (uint8)MessageIDTypes::ID_SCENES)
		{
			return false;
		}

		unsigned int sceneIndex = sceneHandle - (uint8)MessageIDTypes::ID_SCENES;
		auto scene = _scenes[sceneIndex].lock();
		if (scene)
		{
			packet->metadata()["scene"] = scene.get();
			scene->handleMessage(packet);
			return true;
		}

		return false;
	}
};

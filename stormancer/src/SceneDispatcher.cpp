#include "stormancer.h"

namespace Stormancer
{
	SceneDispatcher::SceneDispatcher(std::shared_ptr<IActionDispatcher> evDispatcher)
		: _scenes((uint32)0xff - (uint32)MessageIDTypes::ID_SCENES + 1, nullptr)
		, _eventDispatcher(evDispatcher)
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

	void SceneDispatcher::addScene(Scene* scene)
	{
		if (scene)
		{
			uint8 index = scene->handle() - (uint8)MessageIDTypes::ID_SCENES;
			_scenes[index] = scene;
		}
	}

	void SceneDispatcher::removeScene(uint8 sceneHandle)
	{
		size_t index = sceneHandle - (uint8)MessageIDTypes::ID_SCENES;
		_scenes[index] = nullptr;
	}

	bool SceneDispatcher::handler_impl(uint8 sceneHandle, Packet_ptr packet)
	{
		if (sceneHandle < (uint8)MessageIDTypes::ID_SCENES)
		{
			return false;
		}

		unsigned int sceneIndex = sceneHandle - (uint8)MessageIDTypes::ID_SCENES;
		auto scene = _scenes[sceneIndex];
		if (scene)
		{
			packet->metadata()["scene"] = scene;
			ILogger::instance()->log(LogLevel::Trace, "sceneDispatcher::handle", "dispatching packet to scene " + scene->id());

			this->_eventDispatcher->post([scene,packet]() {scene->handleMessage(packet); });
			return true;
		}
		ILogger::instance()->log(LogLevel::Trace, "sceneDispatcher::handle", "no scene found to dispatch packet handle=" + std::to_string(sceneHandle));

		return false;
	}
};

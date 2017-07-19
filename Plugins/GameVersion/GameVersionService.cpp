#include <stdafx.h>
#include "GameVersionService.h"

namespace Stormancer
{
	GameVersionService::GameVersionService(Scene* scene)
	{
		if (!scene)
		{
			throw std::runtime_error("The scene is invalid.");
		}

		scene->addRoute("gameVersion.update", [this](Packetisp_ptr packet) {
			msgpack::unpacked result;
			std::string buffer;
			*packet->stream >> buffer;

			msgpack::unpack(result, buffer.data(), buffer.size());
			std::string version;
			result.get().convert(&version);

			_gameVersion = version;

			if (_onGameVersionUpdate)
			{
				_onGameVersionUpdate(version);
			}
		});

		scene->addRoute("serverVersion.update", [this](Packetisp_ptr packet) {
			msgpack::unpacked result;
			std::string buffer;
			*packet->stream >> buffer;

			msgpack::unpack(result, buffer.data(), buffer.size());
			std::string version;
			result.get().convert(&version);
			
			if (_onServerVersionUpdate)
			{
				_onServerVersionUpdate(version);
			}
		});
	}

	GameVersionService::~GameVersionService()
	{
	}

	std::string GameVersionService::gameVersion() const
	{
		return _gameVersion;
	}

	void GameVersionService::onGameVersionUpdate(std::function<void(std::string)> callback)
	{
		_onGameVersionUpdate = callback;
	}

	void GameVersionService::onServerVersionUpdate(std::function<void(std::string)> callback)
	{
		_onServerVersionUpdate = callback;
	}
}

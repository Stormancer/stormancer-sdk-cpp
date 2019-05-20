#include "GameVersion/GameVersionService.h"


namespace Stormancer
{
	GameVersionService::GameVersionService(std::shared_ptr<Scene> scene)
	{
		if (!scene)
		{
			throw std::runtime_error("The scene is invalid.");
		}

		scene->addRoute("gameVersion.update", [this](Packetisp_ptr packet) {
			Serializer serializer;
			_gameVersion = serializer.deserializeOne<std::string>(packet->stream);

			if (_onGameVersionUpdate)
			{
				_onGameVersionUpdate(_gameVersion);
			}
		});

		scene->addRoute("serverVersion.update", [this](Packetisp_ptr packet) {
			Serializer serializer;
			std::string version = serializer.deserializeOne<std::string>(packet->stream);

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

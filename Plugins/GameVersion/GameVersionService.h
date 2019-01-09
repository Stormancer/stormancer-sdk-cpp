#pragma once

#include "stormancer/headers.h"
#include "stormancer/Scene.h"

namespace Stormancer
{
	class GameVersionService
	{
	public:

#pragma region public_methods

		GameVersionService(std::shared_ptr<Scene> scene);
		~GameVersionService();
		std::string gameVersion() const;
		void onGameVersionUpdate(std::function<void(std::string)> callback);
		void onServerVersionUpdate(std::function<void(std::string)> callback);

#pragma endregion

	private:

#pragma region private_members

		std::string _gameVersion = "unknown";
		std::function<void(std::string)> _onGameVersionUpdate;
		std::function<void(std::string)> _onServerVersionUpdate;

#pragma endregion
	};
}

#if defined(STORMANCER_CUSTOM_PCH)
#include STORMANCER_CUSTOM_PCH
#endif
#include "PlayerData/PlayerDataService.h"

namespace Stormancer
{
	const ServiceOptions PlayerDataService::service_options {
		ServiceContextFlags::Scene | ServiceContextFlags::CreateWithScene | ServiceContextFlags::SingleInstance,
		"stormancer.playerdata"
	};

	bool PlayerDataService::GetPlayerData(const std::string& stormancerId, const std::string& dataKey, std::vector<byte>& outData)
	{
		std::lock_guard<std::mutex> lg(_dataMutex);

		auto entry = _dataCache[stormancerId][dataKey];
		if (entry.Version <= 0)
		{
			// Version <= 0 means data is not present
			return false;
		}
		outData = entry.Data;
		return true;
	}

	pplx::task<void> PlayerDataService::SetPlayerData(const std::string dataKey, const std::vector<byte>& data)
	{
		auto scene = _scene.lock();
		if (!scene)
		{
			return pplx::task_from_exception<void>(std::runtime_error("PlayerDataService::SetPlayerData: scene was deleted"));
		}

		auto rpc = scene->dependencyResolver().resolve<RpcService>();
		return rpc->rpc<void>(SET_PLAYERDATA_RPC, dataKey, data);
	}

	void PlayerDataService::setScene(std::shared_ptr<Scene> scene)
	{
		_scene = scene;
		_logger = scene->dependencyResolver().resolve<ILogger>();

		std::weak_ptr<PlayerDataService> weakThis = shared_from_this();
		scene->addRoute(PLAYERDATA_UPDATED_ROUTE, [weakThis](Packetisp_ptr packet)
		{
			auto strongThis = weakThis.lock();
			if (strongThis)
			{
				strongThis->onUpdateReceived(packet);
			}
		});
	}

	void PlayerDataService::onUpdateReceived(Packetisp_ptr packet)
	{
		auto updates = packet->readObject<std::vector<PlayerDataUpdate>>();
		for (const auto& update : updates)
		{
			{
				std::lock_guard<std::mutex> lg(_dataMutex);

				// Make sure we aren't overwriting a more recent version of the data
				auto existingOrNewEntry = _dataCache[update.PlayerId][update.DataKey];
				if (existingOrNewEntry.Version > update.Data.Version)
				{
					_logger->log(LogLevel::Trace, "PlayerDataService::onUpdateReceived", "received outdated data, dropping");
					return;
				}
				_dataCache[update.PlayerId][update.DataKey] = update.Data;
			}

			auto strongScene = _scene.lock();
			if (_onDataUpdated && strongScene)
			{
				auto& dr = strongScene->dependencyResolver();
				auto callback = _onDataUpdated;
				dr.resolve<IActionDispatcher>()->post([callback, update] { callback(update); });
			}
		}
	}

}
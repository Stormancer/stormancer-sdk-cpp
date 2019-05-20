#pragma once


#include "stormancer/IService.h"
#include "stormancer/Scene.h"
#include "stormancer/RPC/Service.h"
#include "stormancer/SingleServicePlugin.h"
#include "PlayerDataUpdate.h"
#include <map>

namespace Stormancer
{

class PlayerDataService : public IService, public std::enable_shared_from_this<PlayerDataService>
{
public:

	static const ServiceOptions service_options;

	bool GetPlayerData(const std::string& stormancerId, const std::string& dataKey, std::string& outData);

	pplx::task<void> SetPlayerData(const std::string dataKey, const std::string& data);

	void OnPlayerDataUpdated(std::function<void(const PlayerDataUpdate&)> callback)
	{
		_onDataUpdated = callback;
	}

	virtual void setScene(std::shared_ptr<Scene> scene) override;

private:
	using DataMap = std::map<std::string, std::map<std::string, PlayerDataEntry>>;

	void onUpdateReceived(Packetisp_ptr packet);

	static constexpr const char* PLAYERDATA_UPDATED_ROUTE = "playerdata.updated";
	static constexpr const char* GET_PLAYERDATA_RPC = "playerdata.getplayerdata";
	static constexpr const char* GET_ALL_PLAYERDATA_RPC = "playerdata.getallplayerdata";
	static constexpr const char* SET_PLAYERDATA_RPC = "playerdata.setplayerdata";

	std::weak_ptr<Scene> _scene;
	ILogger_ptr _logger;

	DataMap _dataCache;
	std::mutex _dataMutex;

	std::function<void(const PlayerDataUpdate&)> _onDataUpdated;
};

using PlayerDataPlugin = SingleServicePlugin<PlayerDataService, PlayerDataService::service_options>;

}
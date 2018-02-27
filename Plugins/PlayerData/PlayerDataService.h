#pragma once

#include "stormancer/headers.h"
#include "stormancer/IService.h"
#include "stormancer/Scene.h"
#include "stormancer/RPC/RpcService.h"
#include "stormancer/SingleServicePlugin.h"

namespace Stormancer
{

class PlayerDataService;

ServiceOptions opts{
	ServiceContextFlags::Scene | ServiceContextFlags::CreateWithScene | ServiceContextFlags::SingleInstance,
	"stormancer.playerdata"
};

using PlayerDataPlugin = SingleServicePlugin<PlayerDataService, opts>;

class PlayerDataService : public IService, public std::enable_shared_from_this<PlayerDataService>
{
public:

	template<typename T>
	pplx::task<T> GetPlayerData(const std::string& stormancerId)
	{
		auto rpc = _scene->dependencyResolver()->resolve<RpcService>();

		return rpc->rpc<T>(GET_PLAYERDATA_RPC, stormancerId);
	}

	template<typename T>
	pplx::task<void> SetPlayerData(const T& data)
	{
		auto rpc = _scene->dependencyResolver()->resolve<RpcService>();

		return rpc->rpc<void>(SET_PLAYERDATA_RPC, data);
	}

	void OnPlayerDataUpdated(std::function<void(const std::string&, Packetisp_ptr)> callback)
	{
		_onDataUpdated = callback;
	}

	virtual void setScene(Scene* scene) override
	{
		_scene = scene;

		std::weak_ptr<PlayerDataService> weakThis = shared_from_this();
		_scene->addRoute(PLAYERDATA_UPDATED_ROUTE, [weakThis](Packetisp_ptr packet)
		{
			auto thiz = weakThis.lock();
			if (thiz && thiz->_onDataUpdated)
			{
				const auto stormancerId = packet->readObject<std::string>();
				thiz->_onDataUpdated(stormancerId, packet);
			}
		});
	}

	Scene* GetScene()
	{
		return _scene;
	}

private:

	static constexpr const char* PLAYERDATA_UPDATED_ROUTE = "playerdata.updated";
	static constexpr const char* GET_PLAYERDATA_RPC = "playerdata.getplayerdata";
	static constexpr const char* SET_PLAYERDATA_RPC = "playerdata.setplayerdata";

	Scene* _scene;

	std::function<void(const std::string&, Packetisp_ptr)> _onDataUpdated;
};

}
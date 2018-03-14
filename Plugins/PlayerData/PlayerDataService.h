#pragma once

#include "stormancer/headers.h"
#include "stormancer/IService.h"
#include "stormancer/Scene.h"
#include "stormancer/RPC/RpcService.h"
#include "stormancer/SingleServicePlugin.h"

namespace Stormancer
{
	template<typename T>
	struct PlayerData
	{
		int64 peerId;
		std::string userId;
		T data;

		MSGPACK_DEFINE(peerId, userId, data);
	};

	template<typename T>
	class PlayerDataService : public IService, public std::enable_shared_from_this<PlayerDataService<T>>
	{
	public:

		pplx::task<PlayerData<T>> GetPlayerData(const std::string& userId)
		{
			for (auto it : _playersData)
			{
				if (it.second.userId == userId)
				{
					return pplx::task_from_result(it.second);
				}
			}
			return pplx::task_from_exception<PlayerData<T>>(std::runtime_error("Player data not found."));
		}

		pplx::task<PlayerData<T>> GetPlayerData(int64 peerId)
		{
			if (mapContains(_playersData, peerId))
			{
				return pplx::task_from_result(_playersData.at(peerId));
			}
			return pplx::task_from_exception<PlayerData<T>>(std::runtime_error("Player data not found."));
		}

		pplx::task<void> SetPlayerData(const PlayerData<T>& data)
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

			std::weak_ptr<PlayerDataService<T>> weakThis = this->shared_from_this();
			_scene->addRoute(PLAYERDATA_UPDATED_ROUTE, [weakThis](Packetisp_ptr packet)
			{
				auto thiz = weakThis.lock();
				if (thiz && thiz->_onDataUpdated)
				{
					auto playerData = packet->readObject<PlayerData<T>>();
					playerData.peerId = packet->connection->id();
					thiz->_onDataUpdated(playerData);
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

		std::function<void(PlayerData<T>)> _onDataUpdated;

		std::map<int64, PlayerData<T>> _playersData;
	};

	ServiceOptions opts{
		ServiceContextFlags::Scene | ServiceContextFlags::CreateWithScene | ServiceContextFlags::SingleInstance,
		"stormancer.playerdata"
	};

	template<typename T>
	using PlayerDataPlugin = SingleServicePlugin<PlayerDataService<T>, opts>;
}

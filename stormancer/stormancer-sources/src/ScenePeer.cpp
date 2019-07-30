#include "stormancer/stdafx.h"
#include "stormancer/ScenePeer.h"
#include "stormancer/ChannelUidStore.h"
#include "stormancer/Utilities/PointerUtilities.h"

namespace Stormancer
{
	ScenePeer::ScenePeer(std::weak_ptr<IConnection> connection, byte sceneHandle, std::unordered_map<std::string, Route_ptr>& routes, std::weak_ptr<Scene> scene)
		: _sceneHandle(sceneHandle)
		, _routes(routes)
		, _connection(connection)
		, _scene(scene)
	{
	}

	uint64 ScenePeer::id() const
	{
		return _connection->id();
	}

	std::string ScenePeer::getSceneId() const
	{
		auto scene = _scene.lock();
		if (!scene)
		{
			throw std::runtime_error("Scene destroyed");

		}
		return scene->id();
	}

	std::shared_ptr<IConnection> ScenePeer::connection() const
	{
		return _connection;
	}

	const std::unordered_map<std::string, Route_ptr>& ScenePeer::routes() const
	{
		return _routes;
	}

	void ScenePeer::send(const std::string& routeName, const StreamWriter& streamWriter, PacketPriority priority, PacketReliability reliability, const std::string& channelIdentifier)
	{
		auto scene = LockOrThrow(_scene);
		scene->send(routeName, streamWriter, priority, reliability, channelIdentifier);
	}

	pplx::task<void> ScenePeer::disconnect(pplx::cancellation_token ct)
	{
		auto scene = _scene.lock();
		if (scene)
		{
			return scene->disconnect(ct);
		}
		else
		{
			return pplx::task_from_result(pplx::task_options(ct));
		}
	}

	byte ScenePeer::handle() const
	{
		return _sceneHandle;
	}
}

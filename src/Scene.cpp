#include "stormancer.h"

namespace Stormancer
{
	Scene::Scene(shared_ptr<IConnection> connection, Client client, wstring id, wstring token, SceneInfosDto dto)
		: _id(id),
		_peer(connection),
		_token(token),
		_client(client),
		_metadata(dto.Metadata)
	{
		for (auto routeDto : dto.Routes)
		{
			_remoteRoutesMap[routeDto.Name] = Route(this, routeDto.Name, routeDto.Metadata);
		}
	}

	Scene::~Scene()
	{
	}
};

#include "stormancer.h"

namespace Stormancer
{
	ConnectionResult::ConnectionResult()
	{
	}

	ConnectionResult::~ConnectionResult()
	{
	}

	void ConnectionResult::msgpack_unpack(msgpack::object const& o)
	{
		if (o.type != msgpack::type::MAP)
		{
			throw std::exception("Bad msgpack format");
		}

		auto mapptr = o.via.map.ptr;
		uint32 mapsize = o.via.map.size;

		for (uint32 i = 0; i < mapsize; i++)
		{
			std::string key;
			mapptr[i].key.convert(&key);

			if (key == "SceneHandle")
			{
				mapptr[i].val.convert(&SceneHandle);
			}
			else if (key == "RouteMappings")
			{
				mapptr[i].val.convert(&RouteMappings);
			}
		}
	}
}

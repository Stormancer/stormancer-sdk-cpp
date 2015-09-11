#include "stormancer.h"

namespace Stormancer
{
	SceneInfosDto::SceneInfosDto()
	{
	}

	SceneInfosDto::~SceneInfosDto()
	{
	}

	void SceneInfosDto::msgpack_unpack(msgpack::object const& o)
	{
		if (o.type != msgpack::type::MAP)
		{
			throw std::invalid_argument("Bad msgpack format");
		}

		auto mapptr = o.via.map.ptr;
		uint32 mapsize = o.via.map.size;
		for (uint32 i = 0; i < mapsize; i++)
		{
			std::string key;
			mapptr[i].key.convert(&key);

			if (key == "SceneId")
			{
				mapptr[i].val.convert(&SceneId);
			}
			else if (key == "Metadata")
			{
				mapptr[i].val.convert(&Metadata);
			}
			else if (key == "Routes")
			{
				mapptr[i].val.convert(&Routes);
			}
			else if (key == "SelectedSerializer")
			{
				mapptr[i].val.convert(&SelectedSerializer);
			}
		}
	}
}

#include "stdafx.h"
#include "RouteDto.h"

namespace Stormancer
{
	RouteDto::RouteDto()
	{
	}

	RouteDto::~RouteDto()
	{
	}

	void RouteDto::msgpack_unpack(msgpack::object const& o)
	{
		if (o.type != msgpack::type::MAP)
		{
			throw std::invalid_argument("Bad msgpack format");
		}

		auto mapptr = o.via.map.ptr;
		uint32 mapsize = o.via.map.size;

		for (uint32 i = 0; i < mapsize; i++)
		{
			std::string key2;
			mapptr[i].key.convert(&key2);

			if (key2 == "Handle")
			{
				mapptr[i].val.convert(&Handle);
			}
			else if (key2 == "Metadata")
			{
				mapptr[i].val.convert(&Metadata);
			}
			else if (key2 == "Name")
			{
				mapptr[i].val.convert(&Name);
			}
		}
	}
}

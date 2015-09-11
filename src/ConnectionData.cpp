#include "stormancer.h"

namespace Stormancer
{
	ConnectionData::ConnectionData()
	{
	}

	ConnectionData::~ConnectionData()
	{
	}

	void ConnectionData::msgpack_unpack(msgpack::object const& o)
	{
		if (o.type != msgpack::type::MAP)
		{
			throw "Bad msgpack format";
		}

		auto mapptr = o.via.map.ptr;
		uint32 mapsize = o.via.map.size;
		for (uint32 i = 0; i < mapsize; i++)
		{
			std::string key;
			mapptr[i].key.convert(&key);

			if (key == "AccountId")
			{
				mapptr[i].val.convert(&AccountId);
			}
			else if (key == "Application")
			{
				mapptr[i].val.convert(&Application);
			}
			else if (key == "ContentType")
			{
				mapptr[i].val.convert(&ContentType);
			}
			else if (key == "DeploymentId")
			{
				mapptr[i].val.convert(&DeploymentId);
			}
			else if (key == "Endpoints")
			{
				mapptr[i].val.convert(&Endpoints);
			}
			else if (key == "Expiration")
			{
				int64 ExpirationTmp;
				mapptr[i].val.convert(&ExpirationTmp);
				Expiration = ExpirationTmp;
			}
			else if (key == "Issued")
			{
				int64 IssuedTmp;
				mapptr[i].val.convert(&IssuedTmp);
				Issued = IssuedTmp;
			}
			else if (key == "Routing" && !mapptr[i].val.is_nil())
			{
				mapptr[i].val.convert(&Routing);
			}
			else if (key == "SceneId")
			{
				mapptr[i].val.convert(&SceneId);
			}
			else if (key == "UserData")
			{
				mapptr[i].val.convert(&UserData);
			}
			else if (key == "Version")
			{
				mapptr[i].val.convert(&Version);
			}
		}
	}
}

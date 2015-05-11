#include "stormancer.h"

namespace Stormancer
{
	ConnectionResult::ConnectionResult()
	{
	}

	ConnectionResult::ConnectionResult(bytestream* stream)
	{
		deserialize(stream);
	}

	ConnectionResult::~ConnectionResult()
	{
	}

	void ConnectionResult::serialize(bytestream* stream)
	{
	}

	void ConnectionResult::deserialize(bytestream* stream)
	{
		MsgPack::Deserializer deserializer(stream->rdbuf());
		unique_ptr<MsgPack::Element> element;
		deserializer >> element;

		SceneHandle = numberFromMsgPackMap<uint8>(element, L"SceneHandle");
		RouteMappings = uint16MapFromMsgPackMap(element, L"RouteMappings");
	}
}

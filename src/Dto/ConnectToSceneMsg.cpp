#include "stormancer.h"

namespace Stormancer
{
	ConnectToSceneMsg::ConnectToSceneMsg()
	{
	}

	ConnectToSceneMsg::ConnectToSceneMsg(bytestream* stream)
	{
		deserialize(stream);
	}

	ConnectToSceneMsg::~ConnectToSceneMsg()
	{
	}

	void ConnectToSceneMsg::serialize(bytestream* stream)
	{
		MsgPack::Serializer srlzr(stream->rdbuf());

		srlzr << MsgPack__Factory(MapHeader(3));

		srlzr << MsgPack::Factory("Token");
		srlzr << MsgPack::Factory(Helpers::to_string(Token));

		srlzr << MsgPack::Factory("Routes");
		ISerializable::serializeVector(Routes, stream);

		srlzr << MsgPack::Factory("ConnectionMetadata");
		ISerializable::serialize<stringMap>(ConnectionMetadata, stream);
	}

	void ConnectToSceneMsg::deserialize(bytestream* stream)
	{
	}
}

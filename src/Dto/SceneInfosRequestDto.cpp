#include "stormancer.h"

namespace Stormancer
{
	SceneInfosRequestDto::SceneInfosRequestDto()
	{
	}

	SceneInfosRequestDto::SceneInfosRequestDto(bytestream* stream)
	{
		deserialize(stream);
	}

	SceneInfosRequestDto::~SceneInfosRequestDto()
	{
	}

	void SceneInfosRequestDto::serialize(bytestream* stream)
	{
		MsgPack::Serializer srlzr(stream->rdbuf());

		srlzr << MsgPack__Factory(MapHeader(2));

		srlzr << MsgPack::Factory("Token");
		srlzr << MsgPack::Factory(Helpers::to_string(Token));

		srlzr << MsgPack::Factory("Metadata");
		ISerializable::serialize<stringMap>(Metadata, stream);
	}

	void SceneInfosRequestDto::deserialize(bytestream* stream)
	{
	}
}

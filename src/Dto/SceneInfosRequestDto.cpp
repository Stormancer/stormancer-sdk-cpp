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

		srlzr << MsgPack__Factory(ArrayHeader(2));

		srlzr << MsgPack::Factory(Helpers::to_string(Token));

		ISerializable::serialize<stringMap>(Metadata, stream);
	}

	void SceneInfosRequestDto::deserialize(bytestream* stream)
	{
		// TODO
	}
}

#include "stormancer.h"

namespace Stormancer
{
	SceneInfosDto::SceneInfosDto()
	{
	}

	SceneInfosDto::SceneInfosDto(bytestream* stream)
	{
		deserialize(stream);
	}

	SceneInfosDto::~SceneInfosDto()
	{
	}

	void SceneInfosDto::serialize(bytestream* stream)
	{
	}

	void SceneInfosDto::deserialize(bytestream* stream)
	{
		MsgPack::Deserializer deserializer(stream->rdbuf());
		unique_ptr<MsgPack::Element> element;
		deserializer >> element;

		SceneId = stringFromMsgPackMap(element, L"SceneId");
		Metadata = stringMapFromMsgPackMap(element, L"Metadata");
		Routes = routeDtoVectorFromMsgPackMap(element, L"Routes");
		SelectedSerializer = stringFromMsgPackMap(element, L"SelectedSerializer");
	}
}

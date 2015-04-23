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
		// TODO
	}

	void SceneInfosRequestDto::deserialize(bytestream* stream)
	{
		// TODO
	}
}

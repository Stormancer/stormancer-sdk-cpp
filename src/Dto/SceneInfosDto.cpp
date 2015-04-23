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
		// TODO
	}

	void SceneInfosDto::deserialize(bytestream* stream)
	{
		// TODO
	}
}

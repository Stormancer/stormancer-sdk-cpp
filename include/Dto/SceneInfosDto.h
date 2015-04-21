#pragma once
#include "headers.h"
#include "ISerializable.h"
#include "Dto/RouteDto.h"

namespace Stormancer
{
	struct SceneInfosDto : public ISerializable
	{
	public:
		SceneInfosDto();
		SceneInfosDto(bytestream* stream);
		virtual ~SceneInfosDto();

	public:
		void serialize(bytestream* stream);
		void deserialize(bytestream* stream);

	public:
		wstring SceneId;
		stringMap Metadata;
		vector<RouteDto> Routes;
		wstring SelectedSerializer;
	};
};

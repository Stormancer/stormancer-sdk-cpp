#pragma once
#include "headers.h"
#include "ISerializable.h"

namespace Stormancer
{
	class SceneInfosRequestDto : public ISerializable
	{
	public:
		SceneInfosRequestDto();
		SceneInfosRequestDto(bytestream* stream);
		virtual ~SceneInfosRequestDto();

	public:
		void serialize(bytestream* stream);
		void deserialize(bytestream* stream);

	public:
		wstring Token;
		stringMap Metadata;
	};
};

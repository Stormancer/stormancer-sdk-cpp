#pragma once
#include "headers.h"

namespace Stormancer
{
	struct RouteDto
	{
	public:
		RouteDto();
		RouteDto(bytestream* stream);
		virtual ~RouteDto();

	public:
		void serialize(bytestream* stream);
		void deserialize(bytestream* stream);

	public:
		wstring Name;
		uint16 Handle;
		stringMap Metadata;
	};
};

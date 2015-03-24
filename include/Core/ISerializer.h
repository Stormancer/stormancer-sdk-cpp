#pragma once
#include "headers.h"

namespace Stormancer
{
	class ISerializer
	{
	public:
		ISerializer();
		virtual ~ISerializer();

		virtual string serialize(string data) = 0;
		virtual string deserialize(string bytes) = 0;

	public:
		const string name;
	};
};

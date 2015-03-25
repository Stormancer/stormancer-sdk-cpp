#pragma once
#include "headers.h"

namespace Stormancer
{
	class ISerializer
	{
	public:
		ISerializer();
		virtual ~ISerializer();

		template<typename T>
		void serialize(T data, byteStream& stream)
		{
			throw "Not implemented.";
		}

		template<typename T>
		T deserialize(byteStream& stream)
		{
			throw "Not implemented.";
		}

	public:
		const string name;
	};
};

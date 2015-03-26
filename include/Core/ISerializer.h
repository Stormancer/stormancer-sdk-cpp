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
			throw string("Not implemented.");
		}

		template<typename T>
		T deserialize(byteStream& stream)
		{
			throw string("Not implemented.");
		}

	public:
		const wstring name;
	};
};

#pragma once
#include "headers.h"

namespace Stormancer
{
	class ISerializer
	{
	public:
		ISerializer();
		virtual ~ISerializer();

	public:
		template<typename T>
		void serialize(T data, bytestream* stream)
		{
			throw exception("Not implemented.");
		}

		template<typename T>
		void deserialize(bytestream* stream, T& data)
		{
			throw exception("Not implemented.");
		}

	public:
		const wstring name;
	};
};

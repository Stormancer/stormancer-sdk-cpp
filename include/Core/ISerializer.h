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
			throw string("Not implemented.");
		}

		template<typename T>
		void deserialize(bytestream* stream, T& data)
		{
			throw string("Not implemented.");
		}

	public:
		const wstring name;
	};
};

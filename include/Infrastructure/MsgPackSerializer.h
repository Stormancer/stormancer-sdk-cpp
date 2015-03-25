#pragma once
#include "headers.h"
#include "Core/ISerializer.h"
#include "Infrastructure/MsgPackSerializer.h"

namespace Stormancer
{
	class MsgPackSerializer : public ISerializer
	{
	public:
		MsgPackSerializer();
		~MsgPackSerializer();

		template<typename T>
		void serialize(T data, byteStream& stream)
		{
			MsgPack::Serializer srlz(&stream);
			srlz << MsgPack::Factory(data);
		}

		template<typename T>
		void serialize(T data[], uint32 size, byteStream& stream)
		{
			MsgPack::Serializer srlz(&stream);
			srlz << MsgPack__Factory(ArrayHeader(size));
			for (int i = 0; i < size; i++)
			{
				srlz << MsgPack::Factory(data[i]);
			}
		}

		template<typename T, typename MT>
		void serialize(map<string, MT> myMap)
		{
			MsgPack::Serializer srlz(&stream);
			srlz << MsgPack__Factory(MapHeader(myMap.size()));
			for (auto it = myMap.begin(); it != myMap.end(); ++it)
			{
				srlz << MsgPack::Factory(it->first);
				srlz << MsgPack::Factory(it->second);
			}
		}

		template<typename T>
		T deserialize(byteStream& stream)
		{
			throw "Not implemented.";
		}
	};
};

#pragma once
#include "headers.h"
#include "Core/ISerializer.h"

namespace Stormancer
{
	class MsgPackSerializer : public ISerializer
	{
	public:
		MsgPackSerializer();
		~MsgPackSerializer();

#pragma region serialization

#pragma region Base template

		// Base template
		template<typename T>
		void serialize(T data, byteStream& stream)
		{
			MsgPack::Serializer srlz(&stream);
			srlz << MsgPack::Factory(data);
		}

#pragma endregion Base template
#pragma region Vector template

		// Array template
		template<typename T>
		void serialize(vector<T> data, byteStream& stream)
		{
			MsgPack::Serializer srlz(&stream);
			srlz << MsgPack__Factory(ArrayHeader(data.size()));
			for (int i = 0; i < data.size(); i++)
			{
				srlz << MsgPack::Factory(data[i]);
			}
		}

#pragma endregion Vector template
#pragma region Map template

		// Map template
		template<typename MT>
		void serialize(map<wstring, MT> data, byteStream& stream)
		{
			MsgPack::Serializer srlz(&stream);
			srlz << MsgPack__Factory(MapHeader(myMap.size()));
			for (auto it = data.begin(); it != data.end(); ++it)
			{
				srlz << MsgPack::Factory(it->first);
				//srlz << MsgPack::Factory(it->second);
				serialize<MT>(it->second, stream);
			}
		}

#pragma endregion Map template

#pragma endregion serialization

#pragma region deserialization

#pragma region Base template

		// Base template
		template<typename T>
		T deserialize(byteStream& stream)
		{
			MsgPack::Deserializer dsrlz(&stream);
			unique_ptr<MsgPack::Element> element;
			dsrlz >> element;
			T& data = dynamic_cast<T&>(*element);
			return data;
		}

		// Base template specializations

		// string
		template<>
		string deserialize<string>(byteStream& stream)
		{
			MsgPack::Deserializer dsrlz(&stream);
			unique_ptr<MsgPack::Element> element;
			dsrlz >> element;
			MsgPack::String& str = dynamic_cast<MsgPack::String&>(*element);
			return str.stdString();
		}

		// wstring
		template<>
		wstring deserialize<wstring>(byteStream& stream)
		{
			string str = deserialize<string>(stream);
			return Helpers::to_wstring(str);
		}

		// ConnectionData
		template<>
		ConnectionData deserialize<ConnectionData>(byteStream& stream)
		{
			ConnectionData data;
			data.Endpoints = deserialize<StringMap>(stream);
			data.AccountId = deserialize<wstring>(stream);
			data.Application = deserialize<wstring>(stream);
			data.SceneId = deserialize<wstring>(stream);
			data.Routing = deserialize<wstring>(stream);
			data.Issued = deserialize<time_t>(stream);
			data.Expiration = deserialize<time_t>(stream);
			data.UserData = deserialize<byte*>(stream);
			data.ContentType = deserialize<wstring>(stream);
			return data;
		}

#pragma endregion Base template
#pragma region Vector template

		// Array template
		template<typename VT>
		vector<VT> deserialize(byteStream& stream)
		{
			// TODO
			return vector<VT>();
		}

#pragma endregion Vector template
#pragma region Map template

		// Map template
		template<typename MT>
		map<wstring, MT> deserialize(byteStream& stream)
		{
			MsgPack::Deserializer dsrlz(&stream);
			map<wstring, MT> myMap;
			unique_ptr<MsgPack::MapHeader> mapHeader;
			dsrlz >> mapHeader;
			return myMap;
		}

#pragma endregion Map template

#pragma endregion deserialization
	};
};

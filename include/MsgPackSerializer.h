#pragma once
#include "headers.h"
#include "ISerializer.h"

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
		void serialize(T data, bytestream* stream)
		{
			MsgPack::Serializer srlz(stream->rdbuf());
			srlz << MsgPack::Factory(data);
		}

#pragma endregion
#pragma region Vector template

		// Array template
		template<typename T>
		void serialize(vector<T> data, bytestream* stream)
		{
			MsgPack::Serializer srlz(stream->rdbuf());
			srlz << MsgPack__Factory(ArrayHeader(data.size()));
			for (int i = 0; i < data.size(); i++)
			{
				srlz << MsgPack::Factory(data[i]);
			}
		}

#pragma endregion
#pragma region Map template

		// Map template
		template<typename MT>
		void serialize(map<wstring, MT> data, bytestream* stream)
		{
			MsgPack::Serializer srlz(stream);
			srlz << MsgPack__Factory(MapHeader(myMap.size()));
			for (auto it = data.begin(); it != data.end(); ++it)
			{
				srlz << MsgPack::Factory(it->first);
				//srlz << MsgPack::Factory(it->second);
				serialize<MT>(it->second, stream);
			}
		}

#pragma endregion Map template

#pragma endregion

#pragma region deserialization

#pragma region Base template

		// Base template
		template<typename T>
		void deserialize(bytestream* stream, T& data)
		{
			MsgPack::Deserializer dsrlz(stream->rdbuf());
			unique_ptr<MsgPack::Element> element;
			dsrlz >> element;
			data = dynamic_cast<T&>(*element);
		}

		// Base template specializations

		// string
		template<>
		void deserialize<string>(bytestream* stream, string& str)
		{
			MsgPack::Deserializer dsrlz(stream->rdbuf());
			unique_ptr<MsgPack::Element> element;
			dsrlz >> element;
			str = dynamic_cast<MsgPack::String&>(*element).stdString();
		}

		// wstring
		template<>
		void deserialize<wstring>(bytestream* stream, wstring& str)
		{
			string strTmp;
			deserialize<string>(stream, strTmp);
			str = Helpers::to_wstring(strTmp);
		}

		// ConnectionData
		template<>
		void deserialize<ConnectionData>(bytestream* stream, ConnectionData& data)
		{
			// TODO
		}

#pragma endregion
#pragma region Vector template

		// Array template
		template<typename VT>
		void deserialize(bytestream* stream, vector<VT>& v)
		{
			// TODO
		}

#pragma endregion
#pragma region Map template

		// Map template
		template<typename MT>
		void deserialize(bytestream* stream, map<wstring, MT>& m)
		{
			MsgPack::Deserializer dsrlz(stream);
			unique_ptr<MsgPack::Element> element;
			dsrlz >> element;
			// TODO
		}

#pragma endregion

#pragma endregion
	};
};

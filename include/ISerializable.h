#pragma once
#include "headers.h"

namespace Stormancer
{
	class ISerializable
	{
	public:
		ISerializable();
		ISerializable(bytestream* stream);
		virtual ~ISerializable();

	public:
		virtual void serialize(bytestream* stream) = 0;
		virtual void deserialize(bytestream* stream) = 0;

		// STATIC

		static void serialize(ISerializable* data, bytestream* stream);
		static void deserialize(bytestream* stream, ISerializable* data);

		// DATA (DE)SERIALIZATION
		
		// SERIALIZE

		// Base template ptr
		/*template<typename T>
		static void serialize(T* data, bytestream* stream)
		{
			if (std::is_base_of<ISerializable, T>::value)
			{
				auto* a = dynamic_cast<ISerializable*>(data);
				serialize(a, stream);
			}
			else
			{
				MsgPack::Serializer srlz(stream->rdbuf());
				srlz << MsgPack::Factory(*data);
			}
		}

		template<>
		static void serialize<byte>(byte* data, bytestream* stream);

		template<>
		static void serialize<ISerializable>(ISerializable* data, bytestream* stream);*/

		// Base template ref
		template<typename T>
		static void serialize(T& data, bytestream* stream)
		{
			MsgPack::Serializer srlz(stream->rdbuf());
			srlz << MsgPack::Factory(data);
		}

		// Vector template
		template<typename T>
		static void serialize(vector<T>& data, bytestream* stream)
		{
			MsgPack::Serializer srlz(stream->rdbuf());
			srlz << MsgPack__Factory(ArrayHeader(data.size()));
			for (int i = 0; i < data.size(); i++)
			{
				srlz << MsgPack::Factory(data[i]);
			}
		}

		// Map template
		template<typename MT>
		static void serialize(map<wstring, MT>& data, bytestream* stream)
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

		// DESERIALIZE

		// Base template
		template<typename T>
		static void deserialize(bytestream* stream, T& data)
		{
			MsgPack::Deserializer dsrlz(stream->rdbuf());
			unique_ptr<MsgPack::Element> element;
			dsrlz >> element;
			data = dynamic_cast<T&>(*element);
		}

		// Base template specializations
		// string
		template<>
		static void deserialize<string>(bytestream* stream, string& str);

		// wstring
		template<>
		static void deserialize<wstring>(bytestream* stream, wstring& str);

		// Vector template
		template<typename VT>
		static void deserialize(bytestream* stream, vector<VT>& v)
		{
			// TODO
		}

		// Map template
		template<typename MT>
		static void deserialize(bytestream* stream, map<wstring, MT>& m)
		{
			MsgPack::Deserializer dsrlz(stream);
			unique_ptr<MsgPack::Element> element;
			dsrlz >> element;
			// TODO
		}

		static unique_ptr<MsgPack::Element>& valueFromMsgPackMapKey(unique_ptr<MsgPack::Element>& msgPackMap, wstring key);
		static int64 int64FromMsgPackMap(unique_ptr<MsgPack::Element>& msgPackMap, wstring key);
		static wstring stringFromMsgPackMap(unique_ptr<MsgPack::Element>& msgPackMap, wstring key);
		static stringMap elementToStringMap(MsgPack::Map* msgPackMap);
		static stringMap stringMapFromMsgPackMap(unique_ptr<MsgPack::Element>& msgPackMap, wstring key);
	};
};

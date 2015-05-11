#pragma once
#include "headers.h"
#include "Helpers.h"
#include "Dto/RouteDto.h"

namespace Stormancer
{
	class ISerializable
	{
	public:
		ISerializable();
		virtual ~ISerializable();

	public:
		virtual void serialize(bytestream* stream) = 0;
		virtual void deserialize(bytestream* stream) = 0;

		// STATIC

		static void serialize(ISerializable* data, bytestream* stream);
		static void deserialize(bytestream* stream, ISerializable* data);

		// DATA (DE)SERIALIZATION
		
		// SERIALIZE

		// Base template ref
		template<typename T>
		static void serialize(T& data, bytestream* stream)
		{
			MsgPack::Serializer srlzr(stream->rdbuf());
			srlzr << MsgPack::Factory(data);
		}

		// Base template ref specializations
		template<>
		static void serialize<byte>(byte& data, bytestream* stream)
		{
			MsgPack::Serializer srlzr(stream->rdbuf());
			srlzr << MsgPack::Factory(static_cast<uint64>(data));
		}

		template<>
		static void serialize<string>(string& data, bytestream* stream)
		{
			MsgPack::Serializer srlzr(stream->rdbuf());
			srlzr << MsgPack::Factory(data);
		}

		template<>
		static void serialize<wstring>(wstring& data, bytestream* stream)
		{
			serialize<string>(Helpers::to_string(data), stream);
		}

		template<>
		static void serialize<stringMap>(stringMap& data, bytestream* stream)
		{
			serializeMap<wstring, wstring>(data, stream);
		}

		// Vector template
		template<typename T>
		static void serializeVector(vector<T>& data, bytestream* stream)
		{
			MsgPack::Serializer srlzr(stream->rdbuf());
			srlzr << MsgPack__Factory(ArrayHeader(data.size()));
			for (uint32 i = 0; i < data.size(); i++)
			{
				srlzr << MsgPack::Factory(data[i]);
			}
		}

		template<>
		static void serializeVector(vector<RouteDto>& data, bytestream* stream)
		{
			MsgPack::Serializer srlzr(stream->rdbuf());
			srlzr << MsgPack__Factory(ArrayHeader(data.size()));
			for (uint32 i = 0; i < data.size(); i++)
			{
				data[i].serialize(stream);
			}
		}

		// Map template
		template<typename MT1, typename MT2>
		static void serializeMap(map<MT1, MT2>& data, bytestream* stream)
		{
			MsgPack::Serializer srlzr(stream->rdbuf());
			srlzr << MsgPack__Factory(MapHeader(data.size()));
			for (auto it : data)
			{
				serialize<MT1>(MT1(it.first), stream); // TOIMPROVE (copying object because of key of iterator is const)
				serialize<MT2>(it.second, stream);
			}
		}

		// DESERIALIZE

		// Base template
		template<typename T>
		static void deserialize(bytestream* stream, T& data)
		{
			MsgPack::Deserializer dsrlzr(stream->rdbuf());
			unique_ptr<MsgPack::Element> element;
			dsrlzr >> element;
			data = dynamic_cast<T&>(*element);
		}

		// Base template specializations
		// string
		template<>
		static void deserialize<string>(bytestream* stream, string& str)
		{
			MsgPack::Deserializer dsrlzr(stream->rdbuf());
			unique_ptr<MsgPack::Element> element;
			dsrlzr >> element;
			str = dynamic_cast<MsgPack::String&>(*element).stdString();
		}

		// wstring
		template<>
		static void deserialize<wstring>(bytestream* stream, wstring& str)
		{
			string strTmp;
			deserialize<string>(stream, strTmp);
			str = Helpers::to_wstring(strTmp);
		}

		// Vector template
		template<typename VT>
		static void deserializeVector(bytestream* stream, vector<VT>& v)
		{
			throw string("not implemented.");
		}

		// Map template
		template<typename MT>
		static void deserializeMap(bytestream* stream, map<wstring, MT>& m)
		{
			throw string("not implemented.");
		}

		static unique_ptr<MsgPack::Element>& valueFromMsgPackMapKey(unique_ptr<MsgPack::Element>& msgPackMap, wstring key);

		template<typename T>
		static T numberFromMsgPackMap(unique_ptr<MsgPack::Element>& msgPackMap, wstring key)
		{
			auto& element = valueFromMsgPackMapKey(msgPackMap, key);

			if (auto* number = dynamic_cast<MsgPack::Number*>(element.get()))
			{
				return number->getValue<T>();
			}
			else
			{
				return 0;
			}
		}

		static wstring stringFromMsgPackMap(unique_ptr<MsgPack::Element>& msgPackMap, wstring key);

		static map<wstring, uint16> elementToUInt16Map(MsgPack::Map* msgPackMap);

		static stringMap elementToStringMap(MsgPack::Map* msgPackMap);

		static map<wstring, uint16> uint16MapFromMsgPackMap(unique_ptr<MsgPack::Element>& msgPackMap, wstring key);

		static stringMap stringMapFromMsgPackMap(unique_ptr<MsgPack::Element>& msgPackMap, wstring key);

		static vector<RouteDto> routeDtoVectorFromMsgPackMap(unique_ptr<MsgPack::Element>& msgPackMap, wstring key);
	};
};

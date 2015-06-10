#pragma once
#include "headers.h"
#include "Helpers.h"
#include "Dto/RouteDto.h"

namespace Stormancer
{
	class ISerializable
	{
	public:
		STORMANCER_DLL_API ISerializable();
		STORMANCER_DLL_API virtual ~ISerializable();

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
		static void serialize(byte& data, bytestream* stream);
		static void serialize(string& data, bytestream* stream);
		static void serialize(wstring& data, bytestream* stream);
		static void serialize(stringMap& data, bytestream* stream);

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

		static void serializeVector(vector<RouteDto>& data, bytestream* stream);

		// Map template
		template<typename MT1, typename MT2>
		static void serializeMap(map<MT1, MT2>& data, bytestream* stream)
		{
			MsgPack::Serializer srlzr(stream->rdbuf());
			srlzr << MsgPack__Factory(MapHeader(data.size()));
			for (auto it : data)
			{
				serialize(MT1(it.first), stream); // TOIMPROVE (copying object because of key of iterator is const)
				serialize(it.second, stream);
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
		static void deserialize(bytestream* stream, string& str);

		// wstring
		static void deserialize(bytestream* stream, wstring& str);

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
		static unique_ptr<MsgPack::Element>& valueFromMsgPackArrayKey(unique_ptr<MsgPack::Element>& msgPackArray, uint32 key);

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

		STORMANCER_DLL_API static bool isNullFromMsgPackMap(unique_ptr<MsgPack::Element>& msgPackMap, wstring key);

		STORMANCER_DLL_API static bool boolFromMsgPackMap(unique_ptr<MsgPack::Element>& msgPackMap, wstring key);

		STORMANCER_DLL_API static int8 int8FromMsgPackMap(unique_ptr<MsgPack::Element>& msgPackMap, wstring key);
		STORMANCER_DLL_API static int16 int16FromMsgPackMap(unique_ptr<MsgPack::Element>& msgPackMap, wstring key);
		STORMANCER_DLL_API static int32 int32FromMsgPackMap(unique_ptr<MsgPack::Element>& msgPackMap, wstring key);
		STORMANCER_DLL_API static int64 int64FromMsgPackMap(unique_ptr<MsgPack::Element>& msgPackMap, wstring key);

		STORMANCER_DLL_API static uint8 uint8FromMsgPackMap(unique_ptr<MsgPack::Element>& msgPackMap, wstring key);
		STORMANCER_DLL_API static uint16 uint16FromMsgPackMap(unique_ptr<MsgPack::Element>& msgPackMap, wstring key);
		STORMANCER_DLL_API static uint32 uint32FromMsgPackMap(unique_ptr<MsgPack::Element>& msgPackMap, wstring key);
		STORMANCER_DLL_API static uint64 uint64FromMsgPackMap(unique_ptr<MsgPack::Element>& msgPackMap, wstring key);

		STORMANCER_DLL_API static float floatFromMsgPackMap(unique_ptr<MsgPack::Element>& msgPackMap, wstring key);
		STORMANCER_DLL_API static double doubleFromMsgPackMap(unique_ptr<MsgPack::Element>& msgPackMap, wstring key);

		STORMANCER_DLL_API static wstring stringFromMsgPackMap(unique_ptr<MsgPack::Element>& msgPackMap, wstring key);

		static map<wstring, uint16> elementToUInt16Map(MsgPack::Map* msgPackMap);
		STORMANCER_DLL_API static map<wstring, uint16> uint16MapFromMsgPackMap(unique_ptr<MsgPack::Element>& msgPackMap, wstring key);

		static stringMap elementToStringMap(MsgPack::Map* msgPackMap);
		STORMANCER_DLL_API static stringMap stringMapFromMsgPackMap(unique_ptr<MsgPack::Element>& msgPackMap, wstring key);

		static vector<RouteDto> routeDtoVectorFromMsgPackMap(unique_ptr<MsgPack::Element>& msgPackMap, wstring key);
	};
};

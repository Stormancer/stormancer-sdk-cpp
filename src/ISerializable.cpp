#include "stormancer.h"

namespace Stormancer
{
	ISerializable::ISerializable()
	{
	}

	ISerializable::~ISerializable()
	{
	}


	void ISerializable::serialize(ISerializable* serializable, bytestream* stream)
	{
		serializable->serialize(stream);
	}

	void ISerializable::deserialize(bytestream* stream, ISerializable* serializable)
	{
		serializable->deserialize(stream);
	}


	void ISerializable::serialize(byte& data, bytestream* stream)
	{
		MsgPack::Serializer srlzr(stream->rdbuf());
		srlzr << MsgPack::Factory(static_cast<uint64>(data));
	}

	void ISerializable::serialize(string& data, bytestream* stream)
	{
		MsgPack::Serializer srlzr(stream->rdbuf());
		srlzr << MsgPack::Factory(data);
	}

	void ISerializable::serialize(wstring& data, bytestream* stream)
	{
		serialize<string>(Helpers::to_string(data), stream);
	}

	void ISerializable::serialize(stringMap& data, bytestream* stream)
	{
		serializeMap<wstring, wstring>(data, stream);
	}


	void ISerializable::serializeVector(vector<RouteDto>& data, bytestream* stream)
	{
		MsgPack::Serializer srlzr(stream->rdbuf());
		srlzr << MsgPack__Factory(ArrayHeader(data.size()));
		for (uint32 i = 0; i < data.size(); i++)
		{
			data[i].serialize(stream);
		}
	}


	void ISerializable::deserialize(bytestream* stream, string& str)
	{
		MsgPack::Deserializer dsrlzr(stream->rdbuf());
		unique_ptr<MsgPack::Element> element;
		dsrlzr >> element;
		str = dynamic_cast<MsgPack::String&>(*element).stdString();
	}

	void ISerializable::deserialize(bytestream* stream, wstring& str)
	{
		string strTmp;
		deserialize<string>(stream, strTmp);
		str = Helpers::to_wstring(strTmp);
	}


	unique_ptr<MsgPack::Element>& ISerializable::valueFromMsgPackMapKey(unique_ptr<MsgPack::Element>& msgPackMap, wstring key)
	{
		auto v = dynamic_cast<MsgPack::Map*>(msgPackMap.get())->getContainer();

		if (v->size() % 2)
		{
			throw exception("ISerializable: valueFromMsgPackMapKey error: size must be multiple of 2.");
		}

		for (uint32 i = 0; i < v->size(); i += 2)
		{
			auto key2 = Helpers::to_wstring(dynamic_cast<MsgPack::String*>(v->at(i).get())->stdString());
			if (key2 == key)
			{
				return v->at(i + 1);
			}
		}

		throw exception("ISerializable: valueFromMsgPackMapKey error: Not found.");
	}

	bool ISerializable::isNullFromMsgPackMap(unique_ptr<MsgPack::Element>& msgPackMap, wstring key)
	{
		auto& element = valueFromMsgPackMapKey(msgPackMap, key);

		return (element->getType() == 0xC0);
	}

	bool ISerializable::boolFromMsgPackMap(unique_ptr<MsgPack::Element>& msgPackMap, wstring key)
	{
		auto& element = valueFromMsgPackMapKey(msgPackMap, key);

		if (auto* prim = dynamic_cast<MsgPack::Primitive*>(element.get()))
		{
			return prim->getValue();
		}

		throw exception("ISerializable: This element is not a primitive");
	}

	int8 ISerializable::int8FromMsgPackMap(unique_ptr<MsgPack::Element>& msgPackMap, wstring key)
	{
		auto& element = valueFromMsgPackMapKey(msgPackMap, key);

		if (auto* numb = dynamic_cast<MsgPack::Number*>(element.get()))
		{
			return numb->getValue<int8>();
		}

		throw exception("ISerializable: This element is not a number");
	}

	int16 ISerializable::int16FromMsgPackMap(unique_ptr<MsgPack::Element>& msgPackMap, wstring key)
	{
		auto& element = valueFromMsgPackMapKey(msgPackMap, key);

		if (auto* numb = dynamic_cast<MsgPack::Number*>(element.get()))
		{
			return numb->getValue<int16>();
		}

		throw exception("ISerializable: This element is not a number");
	}

	int32 ISerializable::int32FromMsgPackMap(unique_ptr<MsgPack::Element>& msgPackMap, wstring key)
	{
		auto& element = valueFromMsgPackMapKey(msgPackMap, key);

		if (auto* numb = dynamic_cast<MsgPack::Number*>(element.get()))
		{
			return numb->getValue<int32>();
		}

		throw exception("ISerializable: This element is not a number");
	}

	int64 ISerializable::int64FromMsgPackMap(unique_ptr<MsgPack::Element>& msgPackMap, wstring key)
	{
		auto& element = valueFromMsgPackMapKey(msgPackMap, key);

		if (auto* numb = dynamic_cast<MsgPack::Number*>(element.get()))
		{
			return numb->getValue<int64>();
		}

		throw exception("ISerializable: This element is not a number");
	}

	uint8 ISerializable::uint8FromMsgPackMap(unique_ptr<MsgPack::Element>& msgPackMap, wstring key)
	{
		auto& element = valueFromMsgPackMapKey(msgPackMap, key);

		if (auto* numb = dynamic_cast<MsgPack::Number*>(element.get()))
		{
			return numb->getValue<uint8>();
		}

		throw exception("ISerializable: This element is not a number");
	}

	uint16 ISerializable::uint16FromMsgPackMap(unique_ptr<MsgPack::Element>& msgPackMap, wstring key)
	{
		auto& element = valueFromMsgPackMapKey(msgPackMap, key);

		if (auto* numb = dynamic_cast<MsgPack::Number*>(element.get()))
		{
			return numb->getValue<uint16>();
		}

		throw exception("ISerializable: This element is not a number");
	}

	uint32 ISerializable::uint32FromMsgPackMap(unique_ptr<MsgPack::Element>& msgPackMap, wstring key)
	{
		auto& element = valueFromMsgPackMapKey(msgPackMap, key);

		if (auto* numb = dynamic_cast<MsgPack::Number*>(element.get()))
		{
			return numb->getValue<uint32>();
		}

		throw exception("ISerializable: This element is not a number");
	}

	uint64 ISerializable::uint64FromMsgPackMap(unique_ptr<MsgPack::Element>& msgPackMap, wstring key)
	{
		auto& element = valueFromMsgPackMapKey(msgPackMap, key);

		if (auto* numb = dynamic_cast<MsgPack::Number*>(element.get()))
		{
			return numb->getValue<uint64>();
		}

		throw exception("ISerializable: This element is not a number");
	}

	float ISerializable::floatFromMsgPackMap(unique_ptr<MsgPack::Element>& msgPackMap, wstring key)
	{
		auto& element = valueFromMsgPackMapKey(msgPackMap, key);

		if (auto* numb = dynamic_cast<MsgPack::Number*>(element.get()))
		{
			return numb->getValue<float>();
		}

		throw exception("ISerializable: This element is not a number");
	}

	double ISerializable::doubleFromMsgPackMap(unique_ptr<MsgPack::Element>& msgPackMap, wstring key)
	{
		auto& element = valueFromMsgPackMapKey(msgPackMap, key);

		if (auto* numb = dynamic_cast<MsgPack::Number*>(element.get()))
		{
			return numb->getValue<double>();
		}

		throw exception("ISerializable: This element is not a number");
	}

	wstring ISerializable::stringFromMsgPackMap(unique_ptr<MsgPack::Element>& msgPackMap, wstring key)
	{
		auto& element = valueFromMsgPackMapKey(msgPackMap, key);

		if (auto* str = dynamic_cast<MsgPack::String*>(element.get()))
		{
			return Helpers::to_wstring(str->stdString());
		}

		throw exception("ISerializable: This element is not a string");
	}

	map<wstring, uint16> ISerializable::elementToUInt16Map(MsgPack::Map* msgPackMap)
	{
		map<wstring, uint16> myMap;

		auto v = msgPackMap->getContainer();

		if (v->size() % 2)
		{
			throw exception("ISerializable: elementToStringMap error: size must be multiple of 2.");
		}

		for (uint32 i = 0; i < v->size(); i += 2)
		{
			wstring key = Helpers::to_wstring(((MsgPack::String*)v->at(i).get())->stdString());
			uint16 value = ((MsgPack::Number*)v->at(i + 1).get())->getValue<uint16>();
			myMap[key] = value;
		}

		return myMap;
	}

	stringMap ISerializable::elementToStringMap(MsgPack::Map* msgPackMap)
	{
		stringMap strMap;

		auto v = msgPackMap->getContainer();

		if (v->size() % 2)
		{
			throw exception("ISerializable: elementToStringMap error: size must be multiple of 2.");
		}

		for (uint32 i = 0; i < v->size(); i += 2)
		{
			wstring key = Helpers::to_wstring(((MsgPack::String*)v->at(i).get())->stdString());
			wstring value = Helpers::to_wstring(((MsgPack::String*)v->at(i + 1).get())->stdString());
			strMap[key] = value;
		}

		return strMap;
	}

	map<wstring, uint16> ISerializable::uint16MapFromMsgPackMap(unique_ptr<MsgPack::Element>& msgPackElement, wstring key)
	{
		auto& element = valueFromMsgPackMapKey(msgPackElement, key);

		if (auto* msgPackMap = dynamic_cast<MsgPack::Map*>(element.get()))
		{
			return elementToUInt16Map(msgPackMap);
		}
		else
		{
			return map<wstring, uint16>();
		}
	}

	stringMap ISerializable::stringMapFromMsgPackMap(unique_ptr<MsgPack::Element>& msgPackElement, wstring key)
	{
		auto& element = valueFromMsgPackMapKey(msgPackElement, key);

		if (auto* msgPackMap = dynamic_cast<MsgPack::Map*>(element.get()))
		{
			return elementToStringMap(msgPackMap);
		}
		else
		{
			return stringMap();
		}
	}

	vector<RouteDto> ISerializable::routeDtoVectorFromMsgPackMap(unique_ptr<MsgPack::Element>& msgPackElement, wstring key)
	{
		auto& element = valueFromMsgPackMapKey(msgPackElement, key);

		vector<RouteDto> vres;

		if (auto* arr = dynamic_cast<MsgPack::Array*>(element.get()))
		{
			auto* vdata = arr->getContainer();

			for (uint32 i = 0; i < vdata->size(); i++)
			{
				vres.push_back(RouteDto(vdata->at(i).get()));
			}

		}

		return vres;
	}
};

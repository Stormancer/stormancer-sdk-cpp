#include "stormancer.h"

namespace Stormancer
{
	MsgPackSerializer::MsgPackSerializer()
	{
	}

	MsgPackSerializer::~MsgPackSerializer()
	{
	}

	unique_ptr<MsgPack::Element>& MsgPackSerializer::valueFromMsgPackMapKey(unique_ptr<MsgPack::Element>& msgPackMap, wstring key)
	{
		auto v = dynamic_cast<MsgPack::Map*>(msgPackMap.get())->getContainer();

		if (v->size() % 2)
		{
			throw exception("valueFromMsgPackMapKey error: size must be multiple of 2.");
		}

		for (uint32 i = 0; i < v->size(); i += 2)
		{
			if (Helpers::to_wstring(dynamic_cast<MsgPack::String*>(v->at(i).get())->stdString()) == key)
			{
				return v->at(i + 1);
			}
		}

		throw exception("valueFromMsgPackMapKey error: Not found.");
	}

	int64 MsgPackSerializer::int64FromMsgPackMap(unique_ptr<MsgPack::Element>& msgPackMap, wstring key)
	{
		auto& element = valueFromMsgPackMapKey(msgPackMap, key);

		if (auto* number = dynamic_cast<MsgPack::Number*>(element.get()))
		{
			return number->getValue<int64>();
		}
		else
		{
			return 0;
		}
	}

	wstring MsgPackSerializer::stringFromMsgPackMap(unique_ptr<MsgPack::Element>& msgPackMap, wstring key)
	{
		auto& element = valueFromMsgPackMapKey(msgPackMap, key);

		if (auto* str = dynamic_cast<MsgPack::String*>(element.get()))
		{
			return Helpers::to_wstring(str->stdString());
		}
		else
		{
			return L"";
		}
	}

	stringMap MsgPackSerializer::elementToStringMap(MsgPack::Map* msgPackMap)
	{
		stringMap strMap;

		auto v = msgPackMap->getContainer();

		if (v->size() % 2)
		{
			throw exception("elementToStringMap error: size must be multiple of 2.");
		}

		for (uint32 i = 0; i < v->size(); i += 2)
		{
			wstring key = Helpers::to_wstring(((MsgPack::String*)v->at(i).get())->stdString());
			wstring value = Helpers::to_wstring(((MsgPack::String*)v->at(i + 1).get())->stdString());
			strMap[key] = value;
		}

		return strMap;
	}

	stringMap MsgPackSerializer::stringMapFromMsgPackMap(unique_ptr<MsgPack::Element>& msgPackMap, wstring key)
	{
		auto& element = valueFromMsgPackMapKey(msgPackMap, key);

		if (auto* strMap = dynamic_cast<MsgPack::Map*>(element.get()))
		{
			return elementToStringMap(strMap);
		}
		else
		{
			return stringMap();
		}
	}
};

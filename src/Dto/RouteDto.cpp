#include "stormancer.h"

namespace Stormancer
{
	RouteDto::RouteDto()
	{
	}

	RouteDto::RouteDto(bytestream* stream)
	{
		deserialize(stream);
	}

	RouteDto::RouteDto(MsgPack::Element* element)
	{
		if (element)
		{
			if (auto* m = dynamic_cast<MsgPack::Map*>(element))
			{
				auto* v = m->getContainer();
				for (uint32 i = 0; i + 1 < v->size(); i += 2)
				{
					auto* keyPtr = dynamic_cast<MsgPack::String*>(v->at(i).get());
					if (keyPtr)
					{
						auto key = keyPtr->stdString();
						if (key == "Handle")
						{
							if (auto* n = dynamic_cast<MsgPack::Number*>(v->at(i + 1).get()))
							{
								Handle = n->getValue<uint16>();
							}
						}
						else if (key == "Metadata")
						{
							if (auto* m = dynamic_cast<MsgPack::Map*>(v->at(i + 1).get()))
							{
								Metadata = ISerializable::elementToStringMap(m);
							}
						}
						else if (key == "Name")
						{
							if (auto* s = dynamic_cast<MsgPack::String*>(v->at(i + 1).get()))
							{
								Name = Helpers::to_wstring(s->stdString());
							}
						}
					}
				}
			}
		}
	}

	RouteDto::~RouteDto()
	{
	}

	void RouteDto::serialize(bytestream* stream)
	{
		MsgPack::Serializer srlzr(stream->rdbuf());

		srlzr << MsgPack__Factory(MapHeader(3));

		srlzr << MsgPack::Factory("Name");
		srlzr << MsgPack::Factory(Helpers::to_string(Name));

		srlzr << MsgPack::Factory("Handle");
		srlzr << MsgPack::Factory((uint64)Handle);

		srlzr << MsgPack::Factory("Metadata");
		ISerializable::serialize(Metadata, stream);
	}

	void RouteDto::deserialize(bytestream* stream)
	{
	}
}
#pragma once
#include "../Core/ISerializer.h"
#include "../MsgPack/MsgPack.h"

class MsgPackSerializer : public ISerializer
{
public:
	MsgPackSerializer();
	~MsgPackSerializer();

	template<typename T> void serialize(T data, std::ostream stream)
	{
		return;
	}

	template<typename T> T deserialize(std::istream stream)
	{
		return T();
	}

	const std::string name;

	//MsgPack::Serializer serializer;
};

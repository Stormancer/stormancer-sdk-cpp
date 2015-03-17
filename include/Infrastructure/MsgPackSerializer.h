#pragma once
#include "../Core/ISerializer.h"

namespace Stormancer
{
	class MsgPackSerializer : public ISerializer
	{
	public:
		MsgPackSerializer();
		~MsgPackSerializer();

		std::string serialize(std::string data);

		std::string deserialize(std::string bytes);
	};
};

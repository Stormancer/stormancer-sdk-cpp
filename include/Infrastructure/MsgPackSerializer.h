#pragma once
#include "../Core/ISerializer.h"

namespace Stormancer
{
	class MsgPackSerializer : public ISerializer
	{
	public:
		MsgPackSerializer();
		~MsgPackSerializer();

		string serialize(string data);

		string deserialize(string bytes);
	};
};

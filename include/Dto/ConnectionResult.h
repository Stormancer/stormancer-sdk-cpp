#pragma once
#include "headers.h"
#include "ISerializable.h"

namespace Stormancer
{
	class ConnectionResult : public ISerializable
	{
	public:
		ConnectionResult();
		ConnectionResult(bytestream* stream);
		virtual ~ConnectionResult();

	public:
		void serialize(bytestream* stream);
		void deserialize(bytestream* stream);

	public:
		byte SceneHandle;
		map<wstring, uint16> RouteMappings;
	};
};

#pragma once
#include "headers.h"
#include "ISerializable.h"
#include "Dto/RouteDto.h"

namespace Stormancer
{
	class ConnectToSceneMsg : public ISerializable
	{
	public:
		ConnectToSceneMsg();
		ConnectToSceneMsg(bytestream* stream);
		virtual ~ConnectToSceneMsg();

	public:
		void serialize(bytestream* stream);
		void deserialize(bytestream* stream);

	public:
		wstring Token;
		vector<RouteDto> Routes;
		stringMap ConnectionMetadata;
	};
};

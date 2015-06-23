#pragma once
#include "headers.h"
#include "ISerializable.h"

namespace Stormancer
{
	/// Connection informations to send to the server.
	class ConnectionData : public ISerializable
	{
	public:
		ConnectionData();
		ConnectionData(bytestream* stream);
		virtual ~ConnectionData();

	public:
		void serialize(bytestream* stream);
		void deserialize(bytestream* stream);

	public:
		wstring AccountId;
		wstring Application;
		wstring ContentType;
		wstring DeploymentId;
		stringMap Endpoints;
		time_t Expiration;
		time_t Issued;
		wstring Routing;
		wstring SceneId;
		wstring UserData;
	};
};

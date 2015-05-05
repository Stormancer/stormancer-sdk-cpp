#include "stormancer.h"

namespace Stormancer
{
	ConnectionData::ConnectionData()
	{
	}

	ConnectionData::ConnectionData(bytestream* stream)
	{
		deserialize(stream);
	}

	ConnectionData::~ConnectionData()
	{
	}

	void ConnectionData::serialize(bytestream* stream)
	{
	}

	void ConnectionData::deserialize(bytestream* stream)
	{
		MsgPack::Deserializer deserializer(stream->rdbuf());
		unique_ptr<MsgPack::Element> element;
		deserializer >> element;

		AccountId = stringFromMsgPackMap(element, L"AccountId");
		Application = stringFromMsgPackMap(element, L"Application");
		ContentType = stringFromMsgPackMap(element, L"ContentType");
		DeploymentId = stringFromMsgPackMap(element, L"DeploymentId");
		Endpoints = stringMapFromMsgPackMap(element, L"Endpoints");
		Expiration = int64FromMsgPackMap(element, L"Expiration");
		Issued = int64FromMsgPackMap(element, L"Issued");
		Routing = stringFromMsgPackMap(element, L"Routing");
		SceneId = stringFromMsgPackMap(element, L"SceneId");
		UserData = stringFromMsgPackMap(element, L"UserData");
	}
}

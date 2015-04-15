#include "stormancer.h"

namespace Stormancer
{
	TokenHandler::TokenHandler()
		: _tokenSerializer(new MsgPackSerializer())
	{
	}

	TokenHandler::~TokenHandler()
	{
	}

	SceneEndpoint* TokenHandler::decodeToken(wstring token)
	{
		token = Helpers::stringTrim(token, '"');
		wstring data = Helpers::stringSplit(token, L"-")[0];
		vector<byte> buffer = utility::conversions::from_base64(data);
		string buffer2 = Helpers::to_string(buffer);
		
		auto bs = new bytestream(buffer2);
		MsgPackSerializer* tknMsgPckSrlz = (MsgPackSerializer*)_tokenSerializer;
		if (tknMsgPckSrlz == nullptr)
		{
			throw string("MsgPack serializer not found in TokenHandler::decodeToken");
		}

		auto result = new ConnectionData;
		tknMsgPckSrlz->deserialize(bs, *result);

		auto sceneEp = new SceneEndpoint;
		sceneEp->token = token;
		sceneEp->tokenData = result;
		return sceneEp;
	}
};

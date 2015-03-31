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

	SceneEndpoint TokenHandler::decodeToken(wstring token)
	{
		token = Helpers::stringTrim(token, '"');
		wstring data = Helpers::stringSplit(token, L"-")[0];
		vector<byte> buffer = utility::conversions::from_base64(data);
		string buffer2 = Helpers::to_string(buffer);
		
		byteStream bs(buffer2);
		shared_ptr<MsgPackSerializer> tknMsgPckSrlz = dynamic_pointer_cast<MsgPackSerializer>(_tokenSerializer);
		if (tknMsgPckSrlz.get() == nullptr)
		{
			throw string("MsgPack serializer not found in TokenHandler::decodeToken");
		}

		ConnectionData result;
		tknMsgPckSrlz->deserialize(bs, result);

		SceneEndpoint sceneEp;
		sceneEp.token = token;
		sceneEp.tokenData = result;
		return sceneEp;
	}
};

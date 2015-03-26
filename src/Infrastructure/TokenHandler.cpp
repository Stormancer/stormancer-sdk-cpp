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
		auto buffer = utility::conversions::from_base64(data.c_str());
		string buffer2;
		buffer2.reserve(buffer.size());
		for (auto i = 0; i < buffer.size(); i++)
		{
			buffer2[i] = buffer[i];
		}
		
		byteStream bs(buffer2);
		wstring result = _tokenSerializer->deserialize<wstring>(bs);

		return SceneEndpoint();
	}
};

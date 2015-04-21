#include "stormancer.h"

namespace Stormancer
{
	TokenHandler::TokenHandler()
	{
	}

	TokenHandler::~TokenHandler()
	{
	}

	SceneEndpoint* TokenHandler::decodeToken(wstring& token)
	{
		token = Helpers::stringTrim(token, '"');
		wstring data = Helpers::stringSplit(token, L"-")[0];
		string buffer = Helpers::to_string(utility::conversions::from_base64(data));
		bytestream bs(buffer);

		auto sceneEp = new SceneEndpoint;
		sceneEp->token = token;
		sceneEp->tokenData = new ConnectionData(&bs);
		return sceneEp;
	}
};

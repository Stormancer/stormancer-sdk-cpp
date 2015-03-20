#include "libs.h"
#include "Infrastructure/TokenHandler.h"
#include "Infrastructure/MsgPackSerializer.h"
#include "Helpers.h"

namespace Stormancer
{
	TokenHandler::TokenHandler()
		: _tokenSerializer(make_shared<ISerializer*>(new MsgPackSerializer()))
	{
	}

	TokenHandler::~TokenHandler()
	{
	}

	SceneEndpoint& TokenHandler::decodeToken(string token)
	{
		token = Helpers::stringTrim(token);
		string data = Helpers::stringSplit(token, "-")[0];
		// TODO
	}
};

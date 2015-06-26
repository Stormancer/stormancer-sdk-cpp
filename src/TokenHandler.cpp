#include "stormancer.h"

namespace Stormancer
{
	TokenHandler::TokenHandler()
	{
	}

	TokenHandler::~TokenHandler()
	{
	}

	SceneEndpoint TokenHandler::decodeToken(std::string& token)
	{
		std::wstring wtoken = Helpers::stringTrim(std::wstring(token.begin(), token.end()), L'"');
		std::wstring data = Helpers::stringSplit(wtoken, L"-")[0];
		auto vectorData = utility::conversions::from_base64(data);
		std::string buffer(vectorData.begin(), vectorData.end());

		msgpack::unpacked result;
		msgpack::unpack(result, buffer.data(), buffer.size());
		msgpack::object obj = result.get();

		ConnectionData cData;
		obj.convert(&cData);
		return SceneEndpoint(std::string(wtoken.begin(), wtoken.end()), cData);
	}
};

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
		std::wstring wtoken(token.begin(), token.end());
		wtoken = Helpers::stringTrim(wtoken, L'"');
		std::wstring data = Helpers::stringSplit(wtoken, L"-")[0];
#if defined(_WIN32)
		auto vectorData = utility::conversions::from_base64(data);
#else
		std::string data2(data.begin(), data.end());
		auto vectorData = utility::conversions::from_base64(data2);
#endif
		std::string buffer(vectorData.begin(), vectorData.end());

		msgpack::unpacked result;
		msgpack::unpack(result, buffer.data(), buffer.size());
		msgpack::object obj = result.get();

		ConnectionData cData;
		obj.convert(&cData);
		token = std::string(wtoken.begin(), wtoken.end());
		return SceneEndpoint(token, cData);
	}
};

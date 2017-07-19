#include "stdafx.h"
#include "TokenHandler.h"
#include "Helpers.h"
#include "Logger/ILogger.h"

namespace Stormancer
{
	TokenHandler::TokenHandler()
	{
	}

	TokenHandler::~TokenHandler()
	{
	}

	SceneEndpoint TokenHandler::decodeToken(const std::string& token2)
	{
		if (token2.length() == 0)
		{
			throw std::invalid_argument("Empty token");
		}

		auto token = stringTrim(token2, '"');
		auto data = stringSplit(token, "-")[0];
		utility::string_t data2(data.begin(), data.end());
		auto vectorData = utility::conversions::from_base64(data2);
		std::string buffer(vectorData.begin(), vectorData.end());

		msgpack::unpacked result;
		msgpack::unpack(result, buffer.data(), buffer.size());
		msgpack::object obj = result.get();

		ConnectionData cData;
		obj.convert(&cData);

		std::stringstream ss;
		ss << cData.AccountId
			<< " " << cData.Application
			<< " " << cData.ContentType
			<< " " << cData.DeploymentId
			<< " " << cData.Endpoints.size()
			<< " " << cData.Expiration
			<< " " << cData.Issued
			<< " " << cData.Routing
			<< " " << cData.SceneId
			<< " " << cData.UserData
			<< " " << cData.Version;
		ILogger::instance()->log(LogLevel::Trace, "TokenHandler", "Decoded token : " + ss.str());

		return SceneEndpoint(token, cData);
	}
};

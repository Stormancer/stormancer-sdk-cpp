#include "stormancer/stdafx.h"
#include "stormancer/TokenHandler.h"
#include "stormancer/Helpers.h"
#include "stormancer/Logger/ILogger.h"
#include "stormancer/Serializer.h"

namespace Stormancer
{
	TokenHandler::TokenHandler(ILogger_ptr logger)
		: _logger(logger)
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
		auto data = stringSplit(token, '-')[0];
		utility::string_t data2(data.begin(), data.end());
		auto vectorData = utility::conversions::from_base64(data2);

		ConnectionData cData = _serializer.deserializeOne<ConnectionData>((byte*)vectorData.data(), vectorData.size());

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
		_logger->log(LogLevel::Trace, "TokenHandler", "Decoded token : " + ss.str());

		return SceneEndpoint(token, cData);
	}
};

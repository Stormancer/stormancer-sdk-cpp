#include "stormancer/stdafx.h"
#include "stormancer/TokenHandler.h"
#include "stormancer/Helpers.h"
#include "stormancer/Logger/ILogger.h"
#include "stormancer/Serializer.h"
#include "cpprest/json.h"

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

	SceneEndpoint TokenHandler::getSceneEndpointInfo(const std::string& token)
	{
		auto json = utility::string_t(token.begin(), token.end());
		auto getTokenResponse = web::json::value::parse(json);
		
		SceneEndpoint endpoint;
		endpoint.version = 2;
		endpoint.token = to_string(getTokenResponse[U("token")].as_string());

		endpoint.getTokenResponse.token = to_string(getTokenResponse[U("token")].as_string());

		endpoint.getTokenResponse.encryption.algorithm = to_string(getTokenResponse[U("encryption")][U("algorithm")].as_string());
		endpoint.getTokenResponse.encryption.key = to_string(getTokenResponse[U("encryption")][U("key")].as_string());
		endpoint.getTokenResponse.encryption.mode = to_string(getTokenResponse[U("encryption")][U("mode")].as_string());
		endpoint.getTokenResponse.encryption.token = to_string(getTokenResponse[U("encryption")][U("token")].as_string());

		for (auto transport : getTokenResponse[U("endpoints")].as_object())
		{
			for (auto e : transport.second.as_array())
			{
				endpoint.getTokenResponse.endpoints[to_string(transport.first)].push_back(to_string(e.as_string()));
			}
		}
		return endpoint;
	}

	/*
	public class GetConnectionTokenResponse
    {
        public string token { get; set; }
        public Dictionary<string, List<string>> endpoints { get; set; } = new Dictionary<string, List<string>>();
        public EncryptionConfiguration encryption { get; set; }
    }
    public class EncryptionConfiguration
    {
        public string algorithm { get; set; } = "aes256";
        public string mode { get; set; } = "GCM";
        public string key { get; set; }
        public string token { get; set; }
    }*/
};

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


		return SceneEndpoint(token, cData);
	}

	SceneEndpoint TokenHandler::getSceneEndpointInfo(const std::string& token)
	{
		auto json = utility::string_t(token.begin(), token.end());
		auto getTokenResponse = web::json::value::parse(json);
		
		SceneEndpoint endpoint;
		endpoint.version = 2;
		endpoint.token = to_string(getTokenResponse[_XPLATSTR("token")].as_string());

		endpoint.getTokenResponse.token = to_string(getTokenResponse[_XPLATSTR("token")].as_string());

		endpoint.getTokenResponse.encryption.algorithm = to_string(getTokenResponse[_XPLATSTR("encryption")][_XPLATSTR("algorithm")].as_string());
		endpoint.getTokenResponse.encryption.key = to_string(getTokenResponse[_XPLATSTR("encryption")][_XPLATSTR("key")].as_string());
		endpoint.getTokenResponse.encryption.mode = to_string(getTokenResponse[_XPLATSTR("encryption")][_XPLATSTR("mode")].as_string());
		endpoint.getTokenResponse.encryption.token = to_string(getTokenResponse[_XPLATSTR("encryption")][_XPLATSTR("token")].as_string());

		for (auto transport : getTokenResponse[_XPLATSTR("endpoints")].as_object())
		{
			for (auto e : transport.second.as_array())
			{
				endpoint.getTokenResponse.endpoints[to_string(transport.first)].push_back(to_string(e.as_string()));
			}
		}
		endpoint.tokenData = decodeToken(endpoint.token).tokenData;
		endpoint.version = endpoint.tokenData.Version;
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

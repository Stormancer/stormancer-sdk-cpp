#pragma once

#include "stormancer/BuildConfig.h"


#include "stormancer/ConnectionData.h"

namespace Stormancer
{
	struct EncryptionConfiguration
	{
		std::string algorithm = "aes256";
		std::string mode = "GCM";
		std::string key;
		std::string token;
	};

	struct GetConnectionTokenResponse
	{
		std::string token;
		std::unordered_map<std::string, std::vector<std::string>> endpoints;
		EncryptionConfiguration encryption;
	};


	/// Informations to connect to a scene.
	class SceneEndpoint
	{
	public:
		SceneEndpoint();
		SceneEndpoint(const std::string& token, const ConnectionData& tokenData);
		virtual ~SceneEndpoint();

	public:
		std::string token;
		int version = 1;
		ConnectionData tokenData;
		GetConnectionTokenResponse getTokenResponse;
	};
}

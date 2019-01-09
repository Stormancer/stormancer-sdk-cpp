#pragma once

#include "stormancer/headers.h"
#include "stormancer/Configuration.h"
#include "stormancer/SceneEndpoint.h"
#include "stormancer/ITokenHandler.h"
#include "stormancer/Federation.h"
namespace Stormancer
{
	class ServerEndpoints
	{
	};
	

	/// Retrieve informations from the Api.
	class ApiClient : public std::enable_shared_from_this<ApiClient>
	{
	public:

		ApiClient(std::shared_ptr<ILogger> logger, std::shared_ptr<Configuration> config, std::shared_ptr<ITokenHandler> tokenHandler);
		~ApiClient();
		pplx::task<SceneEndpoint> getSceneEndpoint(std::vector<std::string> endpoints, std::string accountId, std::string applicationName, std::string sceneId, pplx::cancellation_token ct = pplx::cancellation_token::none());
		//Sends a request to the API server, and retries according to the active policy. Parameter is a requestFactory, that provides the targets API endpoint as parameter.
		pplx::task<web::http::http_response> requestWithRetries(std::function<web::http::http_request(std::string)> requestFactory, pplx::cancellation_token ct = pplx::cancellation_token::none());
		pplx::task<ServerEndpoints> GetServerEndpoints(std::vector<std::string> endpoints);

		pplx::task<Federation> getFederation(std::vector<std::string> endpoints, pplx::cancellation_token ct = pplx::cancellation_token::none());
		pplx::task<int> ping(std::string endpoint, pplx::cancellation_token ct = pplx::cancellation_token::none());
	private:
		
		std::shared_ptr<ILogger> _logger;
		pplx::task<SceneEndpoint> getSceneEndpointImpl(std::vector<std::string>  endpoints, std::shared_ptr<std::vector<std::string>> errors, std::string accountId, std::string applicationName, std::string sceneId, pplx::cancellation_token ct = pplx::cancellation_token::none());
		pplx::task<Federation> getFederationImpl(std::vector<std::string>  endpoints, std::shared_ptr<std::vector<std::string>> errors, pplx::cancellation_token ct = pplx::cancellation_token::none());
		pplx::task<web::http::http_response> requestWithRetriesImpl(std::function<web::http::http_request(std::string)> requestFactory, std::vector<std::string> baseUris, std::shared_ptr<std::vector<std::string>> errors, pplx::cancellation_token ct = pplx::cancellation_token::none());
		std::shared_ptr<Configuration> _config;
		std::shared_ptr<ITokenHandler> _tokenHandler;

		Federation readFederationFromJson(std::string json);
	};
};

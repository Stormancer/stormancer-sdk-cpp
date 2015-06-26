#include "stormancer.h"

namespace Stormancer
{
	//using namespace utility;                    // Common utilities like wstring conversions
	using namespace web;                        // Common features like URIs.
	using namespace web::http;                  // Common HTTP functionality
	using namespace web::http::client;          // HTTP client features
	//using namespace concurrency;
	//using namespace concurrency::streams;       // Asynchronous streams
	//using namespace pplx;

	ApiClient::ApiClient(Configuration* config, ITokenHandler* tokenHandler)
		: _config(config),
		_tokenHandler(tokenHandler)
	{
	}

	ApiClient::~ApiClient()
	{
	}

	pplx::task<SceneEndpoint> ApiClient::getSceneEndpoint(std::string accountId, std::string applicationName, std::string sceneId, std::string userData)
	{
		ILogger::instance()->log(LogLevel::Trace, "", "Client::getSceneEndpoint", Helpers::stringFormat(accountId, " ", applicationName, " ", sceneId, " ", userData));

		std::string baseUri = _config->getApiEndpoint();
		http_client client(std::wstring(baseUri.begin(), baseUri.end()));
		http_request request(methods::POST);
		std::string relativeUri = Helpers::stringFormat("/", accountId, "/", applicationName, "/scenes/", sceneId, "/token");
		request.set_request_uri(std::wstring(relativeUri.begin(), relativeUri.end()));
		request.headers().add(L"Content-Type", L"application/msgpack");
		request.headers().add(L"Accept", L"application/json");
		request.headers().add(L"x-version", L"1.0.0");
		request.set_body(std::wstring(userData.begin(), userData.end()));

		return client.request(request).then([this, accountId, applicationName, sceneId](http_response response) {
			uint16 statusCode = response.status_code();
			ILogger::instance()->log(LogLevel::Trace, "", "Client::getSceneEndpoint::client.request", "statusCode = " + std::to_string(statusCode));
			auto ss = new concurrency::streams::stringstreambuf;
			auto result = response.body().read_to_end(*ss).then([this, ss, statusCode, accountId, applicationName, sceneId](size_t size) {
				std::string responseText = ss->collection();
				if (Helpers::ensureSuccessStatusCode(statusCode))
				{
					return _tokenHandler->decodeToken(responseText);
				}
				else
				{
					std::string errorMessage = Helpers::stringFormat("Unable to get scene ", accountId, "/", applicationName, "/", sceneId, ". Please check you used the correct account id, application name and scene id.\nResponse from server:\n", responseText);
					std::stringstream ss;
					ss << errorMessage << std::endl << "Response from server:" << std::endl << responseText;
					throw std::exception(ss.str().c_str());
				}
			});

			result.then([ss](SceneEndpoint t)
			{
				delete ss;
			});

			return result;
		});
	}
};

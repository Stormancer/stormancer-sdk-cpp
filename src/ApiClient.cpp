#include "stormancer.h"

namespace Stormancer
{
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
		ILogger::instance()->log(LogLevel::Trace, "", "Client::getSceneEndpoint", stringFormat(accountId, " ", applicationName, " ", sceneId, " ", userData));

		std::string baseUri = _config->getApiEndpoint();
#if defined(_WIN32)
		web::http::client::http_client client(std::wstring(baseUri.begin(), baseUri.end()));
		web::http::http_request request(web::http::methods::POST);
		std::string relativeUri = stringFormat("/", accountId, "/", applicationName, "/scenes/", sceneId, "/token");
		request.set_request_uri(std::wstring(relativeUri.begin(), relativeUri.end()));
		request.headers().add(L"Content-Type", L"application/msgpack");
		request.headers().add(L"Accept", L"application/json");
		request.headers().add(L"x-version", L"1.0.0");
		request.set_body(std::wstring(userData.begin(), userData.end()));
#else
		web::http::client::http_client client(baseUri);
		web::http::http_request request(web::http::methods::POST);
		std::string relativeUri = stringFormat("/", accountId, "/", applicationName, "/scenes/", sceneId, "/token");
		request.set_request_uri(relativeUri);
		request.headers().add("Content-Type", "application/msgpack");
		request.headers().add("Accept", "application/json");
		request.headers().add("x-version", "1.0.0");
		request.set_body(userData);
#endif

		return client.request(request).then([this, accountId, applicationName, sceneId](web::http::http_response response) {
			uint16 statusCode = response.status_code();
			ILogger::instance()->log(LogLevel::Trace, "", "Client::getSceneEndpoint::client.request", "statusCode = " + to_string(statusCode));
			auto ss = new concurrency::streams::stringstreambuf;
			auto result = response.body().read_to_end(*ss).then([this, ss, statusCode, accountId, applicationName, sceneId](size_t size) {
				std::string responseText = ss->collection();
				if (ensureSuccessStatusCode(statusCode))
				{
					return _tokenHandler->decodeToken(responseText);
				}
				else
				{
					throw std::runtime_error(stringFormat("Unable to get scene ", accountId, "/", applicationName, "/", sceneId, ". Please check you used the correct account id, application name and scene id.\nResponse from server:\n", responseText, "\nResponse from server:\n", responseText).c_str());
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

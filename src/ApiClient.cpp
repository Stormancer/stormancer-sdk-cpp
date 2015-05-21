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
		_createTokenUri(L"{0}/{1}/scenes/{2}/token"),
		_tokenHandler(tokenHandler)
	{
	}

	ApiClient::~ApiClient()
	{
	}

	pplx::task<SceneEndpoint> ApiClient::getSceneEndpoint(wstring accountId, wstring applicationName, wstring sceneId, wstring userData)
	{
		ILogger::instance()->log(LogLevel::Trace, L"", L"Client::getSceneEndpoint", accountId + L" " + applicationName + L" " + sceneId + L" " + userData);

		wstring baseUri = _config->getApiEndpoint();
		http_client client(baseUri);
		http_request request(methods::POST);
		wstring relativeUri = StringFormat(_createTokenUri, accountId, applicationName, sceneId);
		request.set_request_uri(L"/" + relativeUri);
		request.headers().add(L"Content-Type", L"application/msgpack");
		request.headers().add(L"Accept", L"application/json");
		request.headers().add(L"x-version", L"1.0.0");
		request.set_body(userData);

		return client.request(request).then([this, accountId, applicationName, sceneId](http_response response) {
			uint16 statusCode = response.status_code();
			ILogger::instance()->log(LogLevel::Trace, L"", L"Client::getSceneEndpoint::client.request", L"statusCode = " + to_wstring(statusCode));
			auto ss = new concurrency::streams::stringstreambuf;
			auto result = response.body().read_to_end(*ss).then([this, ss, statusCode, accountId, applicationName, sceneId](size_t size) {
				wstring responseText = Helpers::to_wstring(ss->collection());
				if (Helpers::ensureSuccessStatusCode(statusCode))
				{
					return _tokenHandler->decodeToken(responseText);
				}
				else
				{
					string errorMessage(StringFormat(L"Unable to get scene {0}/{1}/{2}. Please check you used the correct account id, application name and scene id.", accountId, applicationName, sceneId));
					stringstream ss;
					ss << errorMessage << endl << "Response from server:" << endl << Helpers::to_string(responseText);
					throw exception(ss.str().c_str());
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

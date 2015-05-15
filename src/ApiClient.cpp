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

	ApiClient::ApiClient(ClientConfiguration* config, ITokenHandler* tokenHandler)
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

		return client.request(request).then([this, accountId, applicationName, sceneId](pplx::task<http_response> t) {
			try
			{
				http_response response = t.get();

				uint16 statusCode = response.status_code();

				ILogger::instance()->log(LogLevel::Trace, L"", L"Client::getSceneEndpoint::client.request", L"statusCode = " + to_wstring(statusCode));

				if (Helpers::ensureSuccessStatusCode(statusCode))
				{
					auto ss = new concurrency::streams::stringstreambuf;
					auto result = response.body().read_to_end(*ss).then([this, ss](pplx::task<size_t> t2) {
						try
						{
							size_t size = t2.get();
							wstring str = Helpers::to_wstring(ss->collection());
							return _tokenHandler->decodeToken(str);
						}
						catch (const exception& e)
						{
							ILogger::instance()->log(e);
							throw e;
						}
					});

					result.then([ss](pplx::task<Stormancer::SceneEndpoint> t3)
					{
						delete ss;
					});

					return result;
				}
				else if (statusCode == 404)
				{
					throw exception(string(StringFormat(L"Unable to get scene {0}/{1}/{2}. Please check you used the correct account id, application name and scene id.", accountId, applicationName, sceneId)).c_str());
				}
				else
				{
					throw exception("Request to get scene failed.");
				}
			}
			catch (const exception& e)
			{
				ILogger::instance()->log(e);
				throw e;
			}
		});
	}
};

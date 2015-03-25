#include "stormancer.h"

namespace Stormancer
{
	//using namespace utility;                    // Common utilities like string conversions
	using namespace web;                        // Common features like URIs.
	using namespace web::http;                  // Common HTTP functionality
	using namespace web::http::client;          // HTTP client features
	//using namespace concurrency;
	//using namespace concurrency::streams;       // Asynchronous streams
	using namespace pplx;

	ApiClient::ApiClient(ClientConfiguration& config/*, shared_ptr<ITokenHandler*> tokenHandler*/)
		: _config(config),
		_createTokenUri("{0}/{1}/scenes/{2}/token")/*,
		_tokenHandler(tokenHandler)*/
	{
	}

	ApiClient::~ApiClient()
	{
	}

	task<SceneEndpoint> ApiClient::getSceneEndpoint(string accountId, string applicationName, string sceneId, string userData)
	{
		MsgPackSerializer serializer;
		byteStream stream;
		serializer.serialize(userData, stream);
		string data = stream.str();

		string relativeUri = Helpers::StringFormat(_createTokenUri, accountId, applicationName, sceneId);
		http_client client(Helpers::to_wstring(_config.getApiEndpoint()));
		http_request request(methods::POST);
		request.headers().add(L"Content-Type", L"application/msgpack");
		request.headers().add(L"Accept", L"application/json");
		request.headers().add(L"x-version", L"1.0.0");

		return client.request(request).then([&](http_response response) {
			cout << Helpers::StringFormat("Server returned status code {0}.", (int)response.status_code());

			web::http::status_code statusCode = response.status_code();
			if (!Helpers::ensureSuccessStatusCode(statusCode))
			{
				throw string(Helpers::StringFormat("Unable to get scene {0}/{1}/{2}. Please check you used the correct account id, application name and scene id.", accountId, applicationName, sceneId));
			}

			//concurrency::streams::streambuf<uint8> sb;
			concurrency::streams::stringstreambuf ss;
			return response.body().read_to_end(ss).then([ss](size_t size) -> SceneEndpoint {
				string str = ss.collection();
				cout << "HOURA!" << str << endl;
				return SceneEndpoint();
			});
		});

		/*return create_task([]() -> SceneEndpoint {
		return SceneEndpoint();
		});*/

		/*pplx::task<web::http::http_response> resp = client.request(, U(relative_uri));
		res.then([=] (pplx::task<web::http_response> task) {
		web::http::http_response response = task.get();
		});*/
	}
};

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

	pplx::task<SceneEndpoint*> ApiClient::getSceneEndpoint(wstring accountId, wstring applicationName, wstring sceneId, wstring userData)
	{
		auto serializer = new MsgPackSerializer;
		byteStream stream;
		serializer->serialize(Helpers::to_string(userData), &stream);
		string data = stream.str();

		//wstring baseUri = _config.getApiEndpoint();
		wstring baseUri = L"http://localhost:8888";
		http_client client(baseUri);
		http_request request(methods::POST);
		request.headers().add(L"Content-Type", L"application/msgpack");
		request.headers().add(L"Accept", L"application/json");
		request.headers().add(L"x-version", L"1.0.0");
		wstring relativeUri = Helpers::StringFormat(_createTokenUri, accountId, applicationName, sceneId);
		request.set_request_uri(L"/" + relativeUri);
		//wstring relativeUri = L"/search?q=Casablanca%20CodePlex";
		//request.set_request_uri(relativeUri);

		return client.request(request).then([this, accountId, applicationName, sceneId](pplx::task<http_response> t) {
			try
			{
				http_response response = t.get();

				uint16 statusCode = response.status_code();
				wcout << Helpers::StringFormat(L"Status code: {0}", statusCode).c_str() << endl;

				if (Helpers::ensureSuccessStatusCode(statusCode))
				{
					auto ss = new concurrency::streams::stringstreambuf;
					return response.body().read_to_end(*ss).then([this, ss](pplx::task<size_t> t2) {
						try
						{
							size_t size = t2.get();
							string strTmp = ss->collection();
							wstring str = Helpers::to_wstring(strTmp);
							wcout << L"HOORA!" << str << endl;
							return _tokenHandler->decodeToken(str);
							//return new SceneEndpoint();
						}
						catch (const exception& e)
						{
							Helpers::displayException(e);
							throw e;
						}
					});
				}
				else if (statusCode == 404)
				{
					throw wstring(Helpers::StringFormat(L"Unable to get scene {0}/{1}/{2}. Please check you used the correct account id, application name and scene id.", accountId, applicationName, sceneId));
				}
			}
			catch (const exception& e)
			{
				Helpers::displayException(e);
				throw e;
			}

			/*cout << Helpers::StringFormat(L"Server returned status code {0}.", (int)response.status_code()).c_str();

			web::http::status_code statusCode = response.status_code();
			if (!Helpers::ensureSuccessStatusCode(statusCode))
			{
				throw wstring(Helpers::StringFormat(L"Unable to get scene {0}/{1}/{2}. Please check you used the correct account id, application name and scene id.", accountId, applicationName, sceneId));
			}

			//concurrency::streams::streambuf<uint8> sb;
			concurrency::streams::stringstreambuf ss;
			return response.body().read_to_end(ss).then([ss](size_t size) -> SceneEndpoint {
				wstring str = Helpers::to_wstring(ss.collection());
				wcout << L"HOURA!" << str << endl;
				return SceneEndpoint();
			});*/
			throw string("Not implemented");
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

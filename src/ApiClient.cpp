#include "libs.h"
#include "ApiClient.h"
#include "Infrastructure/MsgPackSerializer.h"
#include "Helpers.h"

namespace Stormancer
{
	ApiClient::ApiClient(/*ClientConfiguration config, shared_ptr<ITokenHandler*> tokenHandler*/)
		/*: _config(config),
		_createTokenUri("{0}/{1}/scenes/{2}/token"),
		_tokenHandler(tokenHandler)*/
	{
	}

	ApiClient::~ApiClient()
	{
	}

	template<typename T>
	pplx::task<SceneEndpoint> ApiClient::getSceneEndpoint(string accountId, string applicationName, string sceneId, T userData)
	{
		MsgPackSerializer serializer;
		auto data = serializer.serialize(userData);

		string relative_uri = Helpers::stringFormat(_createTokenUri, accountId, applicationName, sceneId);
		web::http::client::http_client client(_config.getApiEndpoint());
		web::http::http_request request(web::http::methods::POST);
		request.headers().add("Content-Type", "application/msgpack");
		request.headers().add("Accept", "application/json");
		request.headers().add("x-version", "1.0.0");
		return client.request(request).then([](web::http::http_response response) {
			wostringstream ss;
			ss << "Server returned status code " << response.status_code() << "." << endl;
			wcout << ss.str();

			web::http::status_code statusCode = response.status_code();
			if (!Helpers::ensureSuccessStatusCode(statusCode))
			{
				throw new exception(Helpers::stringFormat("Unable to get scene {0}/{1}/{2}. Please check you used the correct account id, application name and scene id.");
			}

			response.extract_string().then([=](string body) {
				cout << endl << "BODY:" << endl << body << endl << endl;
			});
		});

		/*pplx::task<web::http::http_response> resp = client.request(, U(relative_uri));
		res.then([=] (pplx::task<web::http_response> task) {
			web::http::http_response response = task.get();
		});*/
	}
};

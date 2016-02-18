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
		{ // TOREMOVE
			std::stringstream ss1;
			ss1 << '[' << accountId.size() << '|' << accountId << ']';
			ss1 << '[' << applicationName.size() << '|' << applicationName << ']';
			ss1 << '[' << sceneId.size() << '|' << sceneId << ']';
			ss1 << '[' << userData.size() << '|' << userData << ']';
			auto str = ss1.str();
			ILogger::instance()->log(LogLevel::Trace, "Client::getSceneEndpoint", "data", str.c_str());
		}

		std::string baseUri = _config->getApiEndpoint();

#if defined(_WIN32)
		web::http::client::http_client client(std::wstring(baseUri.begin(), baseUri.end()));
		web::http::http_request request(web::http::methods::POST);
		std::string relativeUri = std::string("/") + accountId + "/" + applicationName + "/scenes/" + sceneId + "/token";
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

		{ // TOREMOVE
#if defined(_WIN32)
			std::stringstream ss2;
			auto absoluteUri = request.absolute_uri().to_string();
			std::string absoluteUri2(absoluteUri.begin(), absoluteUri.end());
			ss2 << '[' << absoluteUri2.size() << '|' << absoluteUri2 << ']';
			std::string bodyUri;
			auto ss3 = new concurrency::streams::stringstreambuf;
			request.body().read_to_end(*ss3).then([ss3, &bodyUri](size_t size) {
				bodyUri = ss3->collection();
			}).wait();
			ss2 << '[' << bodyUri.size() << '|' << bodyUri << ']';
			auto requestStr = ss2.str();
			ILogger::instance()->log(LogLevel::Trace, "Client::getSceneEndpoint", "request", requestStr.c_str());
#endif
		}

		pplx::task<web::http::http_response> httpRequestTask;
		
		try
		{
			httpRequestTask = client.request(request);
		}
		catch (const std::exception& ex)
		{
			ILogger::instance()->log(LogLevel::Trace, "Client::getSceneEndpoint", "client.request failed.", ex.what());
			return taskFromException<SceneEndpoint>(std::runtime_error(std::string() + "client.request failed." + ex.what()));
		}
		catch (...)
		{
			ILogger::instance()->log(LogLevel::Trace, "Client::getSceneEndpoint", "client.request failed.", "Unknown error");
			return taskFromException<SceneEndpoint>(std::runtime_error(std::string() + "client.request failed."));
		}

		return httpRequestTask.then([](pplx::task<web::http::http_response> t) {
			try
			{
				t.wait();
				return t.get();
			}
			catch (const std::exception& ex)
			{
				throw std::runtime_error(std::string() + ex.what() + "\nCan't reach the stormancer API server.");
			}
			catch (...)
			{
				throw std::runtime_error("Unknown error: Can't reach the stormancer API server.");
			}
		}).then([this, accountId, applicationName, sceneId](web::http::http_response response) {
			try
			{
				uint16 statusCode = response.status_code();
				ILogger::instance()->log(LogLevel::Trace, "Client::getSceneEndpoint", "client.request statusCode", to_string(statusCode).c_str());
				auto ss = new concurrency::streams::stringstreambuf;
				auto result = response.body().read_to_end(*ss).then([this, ss, statusCode, accountId, applicationName, sceneId](size_t size) {
					std::string responseText = ss->collection();
					ILogger::instance()->log(LogLevel::Trace, "Client::getSceneEndpoint", "responseText", responseText.c_str());
					if (ensureSuccessStatusCode(statusCode))
					{
						return _tokenHandler->decodeToken(responseText);
					}
					else
					{
						throw std::runtime_error(std::string("Unable to get scene endpoint ") + accountId + '/' + applicationName + '/' + sceneId + ". Please check your account and application informations.\nResponse from server: " + responseText);
					}
				});

				result.then([ss](pplx::task<SceneEndpoint> t)
				{
					delete ss;
				});

				return result;
			}
			catch (const std::exception& ex)
			{
				throw std::runtime_error(std::string(ex.what()) + "\nCan't get the scene endpoint response.");
			}
		});
	}
};

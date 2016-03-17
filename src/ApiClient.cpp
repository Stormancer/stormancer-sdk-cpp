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

		std::vector<std::string> baseUris = _config->getApiEndpoint();

		std::srand((uint32)std::time(0));

		if (baseUris.size() == 0)
		{
			return taskFromException<SceneEndpoint>(std::runtime_error("No server endpoints available."));
		}

		if (_config->endpointSelectionMode == EndpointSelectionMode::FALLBACK)
		{
			return getSceneEndpointImpl(baseUris, accountId, applicationName, sceneId, userData);
		}
		else if (_config->endpointSelectionMode == EndpointSelectionMode::RANDOM)
		{
			std::vector<std::string> baseUris2;
			while (baseUris.size())
			{
				int index = std::rand() % baseUris.size();
				auto it = baseUris.begin() + index;
				baseUris2.push_back(*it);
				baseUris.erase(it);
			}
			return getSceneEndpointImpl(baseUris2, accountId, applicationName, sceneId, userData);
		}

		return taskFromException<SceneEndpoint>(std::runtime_error("Error selecting server endpoint."));
	}

	pplx::task<SceneEndpoint> ApiClient::getSceneEndpointImpl(std::vector<std::string> endpoints, std::string accountId, std::string applicationName, std::string sceneId, std::string userData)
	{
		if (endpoints.size() == 0)
		{
			return taskFromException<SceneEndpoint>(std::runtime_error("Can't connect to any server endpoint."));
		}

		auto it = endpoints.begin();
		std::string baseUri = *it;
		endpoints.erase(it);

		ILogger::instance()->log(LogLevel::Trace, "Client::getSceneEndpoint", "Get Scene endpoint", baseUri.c_str());

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
			ILogger::instance()->log(LogLevel::Trace, "Client::getSceneEndpoint", "pplx client.request failed.", ex.what());
			return taskFromException<SceneEndpoint>(std::runtime_error(std::string() + "client.request failed." + ex.what()));
		}
		catch (...)
		{
			ILogger::instance()->log(LogLevel::Trace, "Client::getSceneEndpoint", "pplx client.request failed.", "Unknown error");
			return taskFromException<SceneEndpoint>(std::runtime_error(std::string() + "client.request failed."));
		}

		return httpRequestTask.then([this, endpoints, baseUri, accountId, applicationName, sceneId, userData](pplx::task<web::http::http_response> task) {
			web::http::http_response response;
			try
			{
				response = task.get();
			}
			catch (const std::exception& ex)
			{
				auto msgStr = "Can't reach the server endpoint. " + baseUri;
				ILogger::instance()->log(LogLevel::Trace, "Client::getSceneEndpoint", msgStr.c_str(), ex.what());
				//throw std::runtime_error(std::string() + ex.what() + "\nCan't reach the stormancer API server.");
				return getSceneEndpointImpl(endpoints, accountId, applicationName, sceneId, userData);
			}
			catch (...)
			{
				auto msgStr = "Can't reach the server endpoint. " + baseUri;
				ILogger::instance()->log(LogLevel::Trace, "Client::getSceneEndpoint", msgStr.c_str(), "Unknown error");
				//throw std::runtime_error("Unknown error: Can't reach the stormancer API server.");
				return getSceneEndpointImpl(endpoints, accountId, applicationName, sceneId, userData);
			}

			try
			{
				uint16 statusCode = response.status_code();
				auto msgStr = "http request on '" + baseUri + "' returned status code " + std::to_string(statusCode);
				ILogger::instance()->log(LogLevel::Trace, "Client::getSceneEndpoint", msgStr.c_str(), "");
				if (ensureSuccessStatusCode(statusCode))
				{
					auto ss = new concurrency::streams::stringstreambuf;
					return response.body().read_to_end(*ss).then([this, endpoints, ss, statusCode, accountId, applicationName, sceneId, userData](size_t size) {
						std::string responseText = ss->collection();
						ILogger::instance()->log(LogLevel::Trace, "Client::getSceneEndpoint", "responseText", responseText.c_str());
						delete ss;
						return _tokenHandler->decodeToken(responseText);
					});
				}
				else
				{
					return getSceneEndpointImpl(endpoints, accountId, applicationName, sceneId, userData);
				}
			}
			catch (const std::exception& ex)
			{
				throw std::runtime_error(std::string(ex.what()) + "\nCan't get the scene endpoint response.");
			}
		});
	}
};

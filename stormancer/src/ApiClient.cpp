#include "stormancer.h"

namespace Stormancer
{
	ApiClient::ApiClient(std::shared_ptr<Configuration> config, ITokenHandler* tokenHandler)
		: _config(config),
		_tokenHandler(tokenHandler)
	{
	}

	ApiClient::~ApiClient()
	{
	}


	pplx::task<SceneEndpoint> ApiClient::getSceneEndpoint(std::string accountId, std::string applicationName, std::string sceneId)
	{


		std::vector<std::string> baseUris = _config->getApiEndpoint();
		auto errors = std::make_shared<std::vector<std::string>>();
		std::srand((uint32)std::time(0));

		if (baseUris.size() == 0)
		{
			return taskFromException<SceneEndpoint>(std::runtime_error("No server endpoints found in configuration."));
		}

		if (_config->endpointSelectionMode == EndpointSelectionMode::FALLBACK)
		{
			return getSceneEndpointImpl(baseUris, errors, accountId, applicationName, sceneId);
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

			return getSceneEndpointImpl(baseUris2, errors, accountId, applicationName, sceneId);
		}

		return taskFromException<SceneEndpoint>(std::runtime_error("Error selecting server endpoint."));
	}

	pplx::task<SceneEndpoint> ApiClient::getSceneEndpointImpl(std::vector<std::string>  endpoints, std::shared_ptr<std::vector<std::string>> errors, std::string accountId, std::string applicationName, std::string sceneId)
	{
		if (endpoints.size() == 0)
		{
			std::string errorMsg;
			for (auto e : *errors)
			{
				errorMsg = errorMsg + e;
			}

			return taskFromException<SceneEndpoint>(std::runtime_error("Failed to connect to the configured server endpoints : " + errorMsg));
		}

		auto it = endpoints.begin();
		std::string baseUri = *it;
		endpoints.erase(it);

		ILogger::instance()->log(LogLevel::Trace, "Client::getSceneEndpoint", "Get Scene endpoint on : " + baseUri);

#if defined(_WIN32)
		auto config = web::http::client::http_client_config();
		config.set_timeout(std::chrono::seconds(3));
		web::http::client::http_client client(std::wstring(baseUri.begin(), baseUri.end()), config);
		web::http::http_request request(web::http::methods::POST);

		std::string relativeUri = std::string("/") + accountId + "/" + applicationName + "/scenes/" + sceneId + "/token";
		request.set_request_uri(std::wstring(relativeUri.begin(), relativeUri.end()));
		request.headers().add(L"Content-Type", L"application/msgpack");
		request.headers().add(L"Accept", L"application/json");
		request.headers().add(L"x-version", L"1.0.0");
		request.set_body(std::wstring());
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
			ILogger::instance()->log(LogLevel::Trace, "Client::getSceneEndpoint", "sending request : " + requestStr);
#endif
		}

		pplx::task<web::http::http_response> httpRequestTask;

		try
		{
			httpRequestTask = client.request(request);
		}
		catch (const std::exception& ex)
		{
			ILogger::instance()->log(LogLevel::Warn, "Client::getSceneEndpoint", "pplx client.request failed : " + std::string(ex.what()));
			return taskFromException<SceneEndpoint>(std::runtime_error(std::string() + "client.request failed." + ex.what()));
		}
		catch (...)
		{
			ILogger::instance()->log(LogLevel::Warn, "Client::getSceneEndpoint", "pplx client.request failed.");
			return taskFromException<SceneEndpoint>(std::runtime_error(std::string() + "client.request failed."));
		}

		return httpRequestTask.then([this, endpoints, errors, baseUri, accountId, applicationName, sceneId](pplx::task<web::http::http_response> task) {
			web::http::http_response response;
			try
			{
				response = task.get();
			}
			catch (const std::exception& ex)
			{
				auto msgStr = "Can't reach the server endpoint. " + baseUri;
				ILogger::instance()->log(LogLevel::Warn, "Client::getSceneEndpoint", msgStr + " : " + std::string(ex.what()));
				(*errors).push_back("[" + msgStr + ":" + ex.what() + "]");
				//throw std::runtime_error(std::string() + ex.what() + "\nCan't reach the stormancer API server.");
				return getSceneEndpointImpl(endpoints, errors, accountId, applicationName, sceneId);
			}
			catch (...)
			{
				auto msgStr = "Can't reach the server endpoint. " + baseUri;
				ILogger::instance()->log(LogLevel::Warn, "Client::getSceneEndpoint", msgStr);
				(*errors).push_back("[" + msgStr + ":" + "Unknown error]");
				//throw std::runtime_error("Unknown error: Can't reach the stormancer API server.");
				return getSceneEndpointImpl(endpoints, errors, accountId, applicationName, sceneId);
			}

			try
			{
				uint16 statusCode = response.status_code();
				auto msgStr = "http request on '" + baseUri + "' returned status code " + std::to_string(statusCode);
				ILogger::instance()->log(LogLevel::Trace, "Client::getSceneEndpoint", msgStr);
				if (ensureSuccessStatusCode(statusCode))
				{
					auto ss = new concurrency::streams::stringstreambuf;
					return response.body().read_to_end(*ss).then([this, endpoints, ss, statusCode, accountId, applicationName, sceneId](size_t size) {
						std::string responseText = ss->collection();
						ILogger::instance()->log(LogLevel::Trace, "Client::getSceneEndpoint", "response received : " + responseText);
						delete ss;
						return _tokenHandler->decodeToken(responseText);
					});
				}
				else
				{
					(*errors).push_back("[" + msgStr + ":" + std::to_string(statusCode) + "]");
					return getSceneEndpointImpl(endpoints, errors, accountId, applicationName, sceneId);
				}
			}
			catch (const std::exception& ex)
			{
				throw std::runtime_error(std::string(ex.what()) + "\nCan't get the scene endpoint response.");
			}
		});
	}
};

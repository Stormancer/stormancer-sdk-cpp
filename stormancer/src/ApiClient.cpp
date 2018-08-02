#include "stormancer/stdafx.h"
#include "stormancer/ApiClient.h"
#include "stormancer/SafeCapture.h"

namespace Stormancer
{
	ApiClient::ApiClient(std::shared_ptr<ILogger> logger, std::shared_ptr<Configuration> config, std::shared_ptr<ITokenHandler> tokenHandler)
		: _logger(logger)
		, _config(config)
		, _tokenHandler(tokenHandler)
	{
		std::srand((uint32)std::time(0));
	}

	ApiClient::~ApiClient()
	{
	}

	pplx::task<SceneEndpoint> ApiClient::getSceneEndpoint(std::string accountId, std::string applicationName, std::string sceneId, pplx::cancellation_token ct)
	{
		std::stringstream ss;
		ss << accountId << ';' << applicationName << ';' << sceneId;
		_logger->log(LogLevel::Trace, "ApiClient", "Scene endpoint data", ss.str());

		std::vector<std::string> baseUris = _config->getApiEndpoint();
		auto errors = std::make_shared<std::vector<std::string>>();

		if (baseUris.size() == 0)
		{
			return pplx::task_from_exception<SceneEndpoint>(std::runtime_error("No server endpoints found in configuration."));
		}

		if (_config->endpointSelectionMode == EndpointSelectionMode::FALLBACK)
		{
			return getSceneEndpointImpl(baseUris, errors, accountId, applicationName, sceneId, ct);
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
			return getSceneEndpointImpl(baseUris2, errors, accountId, applicationName, sceneId, ct);
		}

		return pplx::task_from_exception<SceneEndpoint>(std::runtime_error("Error selecting server endpoint."));
	}

	pplx::task<SceneEndpoint> ApiClient::getSceneEndpointImpl(std::vector<std::string> endpoints, std::shared_ptr<std::vector<std::string>> errors, std::string accountId, std::string applicationName, std::string sceneId, pplx::cancellation_token ct)
	{
		if (endpoints.size() == 0)
		{
			std::string errorMsg;
			for (auto e : *errors)
			{
				errorMsg = errorMsg + e;
			}

			return pplx::task_from_exception<SceneEndpoint>(std::runtime_error("Failed to connect to the configured server endpoints : " + errorMsg));
		}

		auto it = endpoints.begin();
		std::string baseUri = *it;
		utility::string_t baseUri2(baseUri.begin(), baseUri.end());
		endpoints.erase(it);

		auto config = web::http::client::http_client_config();
		config.set_timeout(std::chrono::seconds(30));


		config.set_initHttpLib(_config->shoudInitializeNetworkLibraries);


		web::http::client::http_client client(baseUri2, config);
		web::http::http_request request(web::http::methods::POST);
		std::string relativeUri = "/" + accountId + "/" + applicationName + "/scenes/" + sceneId + "/token";
		utility::string_t relativeUri2(relativeUri.begin(), relativeUri.end());
		request.set_request_uri(relativeUri2);
		request.set_body(utility::string_t());


		request.headers().add(U("Content-Type"), U("application/msgpack"));
		request.headers().add(U("Accept"), U("application/json"));
		request.headers().add(U("x-version"), U("2"));


		return client.request(request, ct)
			.then([=](pplx::task<web::http::http_response> task)
		{
			web::http::http_response response;
			try
			{
				response = task.get();
			}
			catch (const std::exception& ex)
			{
				auto msgStr = "Can't reach the server endpoint. " + baseUri;
				_logger->log(LogLevel::Warn, "ApiClient", msgStr, ex.what());
				(*errors).push_back("[" + msgStr + ":" + ex.what() + "]");
				return getSceneEndpointImpl(endpoints, errors, accountId, applicationName, sceneId, ct);
			}

			try
			{
				uint16 statusCode = response.status_code();
				auto msgStr = "HTTP request on '" + baseUri + "' returned status code " + std::to_string(statusCode);
				_logger->log(LogLevel::Trace, "ApiClient", msgStr);
				concurrency::streams::stringstreambuf ss;
				return response.body().read_to_end(ss)
					.then([=](size_t)
				{
					
					std::string responseText = ss.collection();
					_logger->log(LogLevel::Trace, "ApiClient", "Response", responseText);

					if (ensureSuccessStatusCode(statusCode))
					{
						auto headers = response.headers();
						if (headers[U("x-version")] == U("2"))
						{
							_logger->log(LogLevel::Trace, "ApiClient", "Get token API version : 2");
							return pplx::task_from_result(_tokenHandler->getSceneEndpointInfo(responseText));
						}
						else
						{
							_logger->log(LogLevel::Trace, "ApiClient", "Get token API version : 1");
							return pplx::task_from_result(_tokenHandler->decodeToken(responseText));
						}
					}
					else
					{
						(*errors).push_back("[" + msgStr + ":" + std::to_string(statusCode) + "]");
						return getSceneEndpointImpl(endpoints, errors, accountId, applicationName, sceneId, ct);
					}
				}, ct);
			}
			catch (const std::exception& ex)
			{
				throw std::runtime_error(std::string() + "Can't get the scene endpoint response: " + ex.what());
			}
		}, ct);
	}

	pplx::task<web::http::http_response> ApiClient::requestWithRetries(std::function<web::http::http_request(std::string)> requestFactory, pplx::cancellation_token ct)
	{
		auto errors = std::make_shared<std::vector<std::string>>();
		std::vector<std::string> baseUris = _config->getApiEndpoint();
		return requestWithRetriesImpl(requestFactory, baseUris, errors, ct);
	}

	pplx::task<ServerEndpoints> ApiClient::GetServerEndpoints()
	{
		auto account = _config->account;
		auto application = _config->application;

		return requestWithRetries([=](std::string) {
			web::http::http_request request(web::http::methods::POST);

			std::string relativeUri = std::string("/") + account + "/" + application + "/_endpoints";
#if defined(_WIN32)
			request.set_request_uri(std::wstring(relativeUri.begin(), relativeUri.end()));
			request.headers().add(L"Accept", L"application/json");
			request.headers().add(L"x-version", L"1.0.0");
			request.set_body(std::wstring());
#else
			request.set_request_uri(relativeUri);
			request.headers().add("Accept", "application/json");
			request.headers().add("x-version", "1.0.0");
			request.set_body(std::string());
#endif
			return request;
		})
			.then([](web::http::http_response response)
		{
			return response.extract_json();
		})
			.then([](web::json::value /*value*/)
		{
			ServerEndpoints result;
			return result;
		});
	}

	pplx::task<web::http::http_response> ApiClient::requestWithRetriesImpl(std::function<web::http::http_request(std::string)> requestFactory, std::vector<std::string> endpoints, std::shared_ptr<std::vector<std::string>> errors, pplx::cancellation_token ct)
	{
		auto it = endpoints.begin();
		if (_config->endpointSelectionMode == Stormancer::EndpointSelectionMode::RANDOM)
		{
			int index = std::rand() % endpoints.size();
			it += index;
		}
		std::string baseUri = *it;
		endpoints.erase(it);

		auto rq = requestFactory(baseUri);

		auto config = web::http::client::http_client_config();
		config.set_timeout(std::chrono::seconds(10));

#if defined(_WIN32)
		web::http::client::http_client client(std::wstring(baseUri.begin(), baseUri.end()), config);
#else
		web::http::client::http_client client(baseUri, config);
#endif

		return client.request(rq, ct)
			.then(createSafeCapture(STRM_WEAK_FROM_THIS(), [=](pplx::task<web::http::http_response> t)
		{
			bool success = false;
			web::http::http_response response;
			try
			{
				response = t.get();
				if (response.status_code() == 200)
				{
					success = true;
				}
				else
				{
					auto error = response.extract_string(true).get();
					errors->push_back("An error occured while performing an http request to" + baseUri + " status code : '" + std::to_string(response.status_code()) + "', message : '" + std::string(error.begin(), error.end()));
				}
			}
			catch (std::exception &ex)
			{
				errors->push_back("An exception occured while performing an http request to" + baseUri + ex.what());
			}
			if (!success)
			{
				if (endpoints.size() > 0)
				{
					return requestWithRetriesImpl(requestFactory, endpoints, errors, ct);
				}
				else
				{
					auto str = std::stringstream();
					for (auto error : *errors)
					{
						str << error << '\n';
					}
					throw std::runtime_error(str.str());
				}
			}
			else
			{
				return pplx::task_from_result(response);
			}
		}), ct);
	}
};

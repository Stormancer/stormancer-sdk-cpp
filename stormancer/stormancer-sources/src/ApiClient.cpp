#include "stormancer/stdafx.h"
#include "stormancer/ApiClient.h"
#include "stormancer/SafeCapture.h"
#include "stormancer/Helpers.h"
#include "cpprest/http_client.h"
#include "stormancer/Utilities/StringUtilities.h"

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

	pplx::task<int> ApiClient::ping(std::string endpoint, pplx::cancellation_token ct)
	{
		utility::string_t baseUri2(endpoint.begin(), endpoint.end());


		auto config = web::http::client::http_client_config();
		config.set_timeout(std::chrono::seconds(30));


		config.set_initHttpLib(_config->shoudInitializeNetworkLibraries);


		web::http::client::http_client client(baseUri2, config);
		web::http::http_request request(web::http::methods::GET);
		std::string relativeUri = "/_federation";
		utility::string_t relativeUri2(relativeUri.begin(), relativeUri.end());
		request.set_request_uri(relativeUri2);
		request.set_body(utility::string_t());

		request.headers().add(_XPLATSTR("Content-Type"), _XPLATSTR("application/msgpack"));
		request.headers().add(_XPLATSTR("Accept"), _XPLATSTR("application/json"));
		request.headers().add(_XPLATSTR("x-version"), _XPLATSTR("3"));

		auto wApiClient = STRM_WEAK_FROM_THIS();
		auto start = std::chrono::high_resolution_clock::now();
		return client.request(request, ct)
			.then([start](web::http::http_response task)
		{
			auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start);

			return (int)duration.count();
		});
	}

	pplx::task<Federation> ApiClient::getFederation(std::vector<std::string> endpoints, pplx::cancellation_token ct)
	{

		auto errors = std::make_shared<std::vector<std::string>>();

		if (endpoints.size() == 0)
		{
			return pplx::task_from_exception<Federation>(std::runtime_error("No server endpoints found in configuration."));
		}

		if (_config->endpointSelectionMode == EndpointSelectionMode::FALLBACK)
		{
			return getFederationImpl(endpoints, errors, ct);
		}
		else if (_config->endpointSelectionMode == EndpointSelectionMode::RANDOM)
		{
			std::vector<std::string> baseUris2;
			while (endpoints.size())
			{
				int index = std::rand() % endpoints.size();
				auto it = endpoints.begin() + index;
				baseUris2.push_back(*it);
				endpoints.erase(it);
			}
			return getFederationImpl(baseUris2, errors, ct);
		}

		return pplx::task_from_exception<Federation>(std::runtime_error("Error selecting server endpoint."));
	}

	pplx::task<Federation> ApiClient::getFederationImpl(std::vector<std::string> endpoints, std::shared_ptr<std::vector<std::string>> errors, pplx::cancellation_token ct)
	{
		if (endpoints.size() == 0)
		{
			std::string errorMsg;
			for (auto e : *errors)
			{
				errorMsg = errorMsg + e;
			}
			std::string message = "Failed to connect to the configured server endpoints : " + errorMsg;
			return pplx::task_from_exception<Federation>(std::runtime_error(message.c_str()));
		}

		auto it = endpoints.begin();
		std::string baseUri = *it;
		utility::string_t baseUri2(baseUri.begin(), baseUri.end());
		endpoints.erase(it);

		auto config = web::http::client::http_client_config();
		config.set_timeout(std::chrono::seconds(30));


		config.set_initHttpLib(_config->shoudInitializeNetworkLibraries);


		web::http::client::http_client client(baseUri2, config);
		web::http::http_request request(web::http::methods::GET);
		std::string relativeUri = "/_federation";
		utility::string_t relativeUri2(relativeUri.begin(), relativeUri.end());
		request.set_request_uri(relativeUri2);
		request.set_body(utility::string_t());

		request.headers().add(_XPLATSTR("Content-Type"), _XPLATSTR("application/msgpack"));
		request.headers().add(_XPLATSTR("Accept"), _XPLATSTR("application/json"));
		request.headers().add(_XPLATSTR("x-version"), _XPLATSTR("3"));

		auto wApiClient = STRM_WEAK_FROM_THIS();

		return client.request(request, ct)
			.then([wApiClient, baseUri, errors, endpoints, ct](pplx::task<web::http::http_response> task)
		{
			auto apiClient = LockOrThrow(wApiClient);

			web::http::http_response response;
			try
			{
				response = task.get();
			}
			catch (const std::exception& ex)
			{
				auto msgStr = "Can't reach the server endpoint. " + baseUri;
				apiClient->_logger->log(LogLevel::Warn, "ApiClient", msgStr, ex.what());
				(*errors).push_back("[" + msgStr + ":" + ex.what() + "]");
				return apiClient->getFederationImpl(endpoints, errors, ct);
			}

			try
			{
				uint16 statusCode = response.status_code();
				auto msgStr = "HTTP request on '" + baseUri + "' returned status code " + std::to_string(statusCode);
				apiClient->_logger->log(LogLevel::Trace, "ApiClient", msgStr);
				concurrency::streams::stringstreambuf ss;
				return response.body().read_to_end(ss)
					.then([wApiClient, ss, statusCode, response, errors, msgStr, endpoints, ct](size_t)
				{
					auto apiClient = LockOrThrow(wApiClient);

					std::string responseText = ss.collection();
					apiClient->_logger->log(LogLevel::Trace, "ApiClient", "Response", responseText);

					if (ensureSuccessStatusCode(statusCode))
					{

						apiClient->_logger->log(LogLevel::Trace, "ApiClient", "Get token API version : 1");
						return pplx::task_from_result(apiClient->readFederationFromJson(responseText));

					}
					else
					{
						(*errors).push_back("[" + msgStr + ":" + std::to_string(statusCode) + "]");
						return apiClient->getFederationImpl(endpoints, errors, ct);
					}
				}, ct);
			}
			catch (const std::exception& ex)
			{
				throw std::runtime_error((std::string() + "Can't get the scene endpoint response: " + ex.what()).c_str());
			}
		}, ct);
	}

	Federation ApiClient::readFederationFromJson(std::string str)
	{
		auto json = utility::conversions::to_string_t(str);
		auto jFed = web::json::value::parse(json);

		Federation f;

		for (auto jCluster : jFed[_XPLATSTR("clusters")].as_array())
		{
			Cluster cluster;
			cluster.id = utility::conversions::to_utf8string(jCluster[_XPLATSTR("id")].as_string());
			for (auto jEndpoint : jCluster[_XPLATSTR("endpoints")].as_array())
			{
				cluster.endpoints.push_back(utility::conversions::to_utf8string(jEndpoint.as_string()));
			}
			for (auto jTag : jCluster[_XPLATSTR("tags")].as_array())
			{
				cluster.tags.push_back(utility::conversions::to_utf8string(jTag.as_string()));
			}
			f.clusters.push_back(cluster);
		}

		//Load data about current cluster
		Cluster current;
		auto jCluster = jFed[_XPLATSTR("current")];
		current.id = utility::conversions::to_utf8string(jCluster[_XPLATSTR("id")].as_string());
		for (auto jEndpoint : jCluster[_XPLATSTR("endpoints")].as_array())
		{
			current.endpoints.push_back(utility::conversions::to_utf8string(jEndpoint.as_string()));
		}
		for (auto jTag : jCluster[_XPLATSTR("tags")].as_array())
		{
			current.tags.push_back(utility::conversions::to_utf8string(jTag.as_string()));
		}
		f.current = current;
		return f;
	}

	pplx::task<SceneEndpoint> ApiClient::getSceneEndpoint(std::vector<std::string> baseUris, std::string accountId, std::string applicationName, std::string sceneId, pplx::cancellation_token ct)
	{
		std::stringstream ss;
		ss << accountId << ';' << applicationName << ';' << sceneId;
		_logger->log(LogLevel::Trace, "ApiClient", "Scene endpoint data", ss.str());

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
			std::string message = "Failed to connect to the configured server endpoints : " + errorMsg;
			return pplx::task_from_exception<SceneEndpoint>(std::runtime_error(message.c_str()));
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

		request.headers().add(_XPLATSTR("Content-Type"), _XPLATSTR("application/msgpack"));
		request.headers().add(_XPLATSTR("Accept"), _XPLATSTR("application/json"));
		request.headers().add(_XPLATSTR("x-version"), _XPLATSTR("3"));

		auto wApiClient = STRM_WEAK_FROM_THIS();

		return client.request(request, ct)
			.then([wApiClient, baseUri, errors, endpoints, accountId, applicationName, sceneId, ct](pplx::task<web::http::http_response> task)
		{
			auto apiClient = LockOrThrow(wApiClient);

			web::http::http_response response;
			try
			{
				response = task.get();
			}
			catch (const std::exception& ex)
			{
				auto msgStr = "Can't reach the server endpoint. " + baseUri;
				apiClient->_logger->log(LogLevel::Warn, "ApiClient", msgStr, ex.what());
				(*errors).push_back("[" + msgStr + ":" + ex.what() + "]");
				return apiClient->getSceneEndpointImpl(endpoints, errors, accountId, applicationName, sceneId, ct);
			}

			try
			{
				uint16 statusCode = response.status_code();
				auto msgStr = "HTTP request on '" + baseUri + "' returned status code " + std::to_string(statusCode);
				apiClient->_logger->log(LogLevel::Trace, "ApiClient", msgStr);
				concurrency::streams::stringstreambuf ss;
				return response.body().read_to_end(ss)
					.then([wApiClient, ss, statusCode, response, errors, msgStr, endpoints, accountId, applicationName, sceneId, ct](size_t)
				{
					auto apiClient = LockOrThrow(wApiClient);

					std::string responseText = ss.collection();
					apiClient->_logger->log(LogLevel::Trace, "ApiClient", "Response", responseText);

					if (ensureSuccessStatusCode(statusCode))
					{
						auto headers = response.headers();
						auto xVersion = headers[_XPLATSTR("x-version")];
						if (xVersion == _XPLATSTR("2") || xVersion == _XPLATSTR("3"))
						{
							apiClient->_logger->log(LogLevel::Trace, "ApiClient", "Get token API version : 2");
							return pplx::task_from_result(apiClient->_tokenHandler->getSceneEndpointInfo(responseText));
						}
						else
						{
							apiClient->_logger->log(LogLevel::Trace, "ApiClient", "Get token API version : 1");
							return pplx::task_from_result(apiClient->_tokenHandler->decodeToken(responseText));
						}
					}
					else
					{
						(*errors).push_back("[" + msgStr + ":" + std::to_string(statusCode) + "]");
						return apiClient->getSceneEndpointImpl(endpoints, errors, accountId, applicationName, sceneId, ct);
					}
				}, ct);
			}
			catch (const std::exception& ex)
			{
				throw std::runtime_error((std::string() + "Can't get the scene endpoint response: " + ex.what()).c_str());
			}
		}, ct);
	}

	pplx::task<web::http::http_response> ApiClient::requestWithRetries(std::function<web::http::http_request(std::string)> requestFactory, pplx::cancellation_token ct)
	{
		auto errors = std::make_shared<std::vector<std::string>>();
		std::vector<std::string> baseUris = _config->getApiEndpoint();
		return requestWithRetriesImpl(requestFactory, baseUris, errors, ct);
	}

	pplx::task<ServerEndpoints> ApiClient::GetServerEndpoints(std::vector<std::string> endpoints)
	{
		auto account = _config->account;
		auto application = _config->application;

		return requestWithRetries([=](std::string) {
			web::http::http_request request(web::http::methods::POST);

			std::string relativeUri = std::string("/") + account + "/" + application + "/_endpoints";
#if defined(_WIN32)
			request.set_request_uri(std::wstring(relativeUri.begin(), relativeUri.end()));
			request.headers().add(L"Accept", L"application/json");
			request.headers().add(L"x-version", L"3");
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

		auto wApiClient = STRM_WEAK_FROM_THIS();

		return client.request(rq, ct)
			.then([wApiClient, baseUri, errors, requestFactory, endpoints, ct](pplx::task<web::http::http_response> t)
		{
			auto apiClient = LockOrThrow(wApiClient);

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
					auto werror = response.extract_string(true).get();
					auto error = utility::conversions::to_utf8string(werror);
					errors->push_back("An error occured while performing an http request to" + baseUri + " status code : '" + std::to_string(response.status_code()) + "', message : '" + error);
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
					return apiClient->requestWithRetriesImpl(requestFactory, endpoints, errors, ct);
				}
				else
				{
					auto str = std::stringstream();
					for (auto error : *errors)
					{
						str << error << '\n';
					}
					throw std::runtime_error(str.str().c_str());
				}
			}
			else
			{
				return pplx::task_from_result(response);
			}
		}, ct);
	}

	Cluster Federation::getCluster(std::string id)
	{
		if (current.id == id)
		{
			return current;
		}
		else
		{
			for (auto cluster : clusters)
			{
				if (cluster.id == id)
				{
					return cluster;
				}
			}
		}
		throw std::runtime_error("Cluster '" + id + "' not found in federation.");
	}
};

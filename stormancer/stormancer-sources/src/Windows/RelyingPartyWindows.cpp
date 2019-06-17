#include "stormancer/stdafx.h"
#include "stormancer/Windows/RelyingPartyWindows.h"
#include "cpprest/http_client.h"

namespace Stormancer
{
	pplx::task<std::string> GetXboxLiveAuthToken(std::string relyingPartyUrl, std::string account, std::string application, std::string token, std::string signature)
	{
		auto config = web::http::client::http_client_config();
		config.set_timeout(std::chrono::seconds(8));
		web::http::client::http_client client(utility::conversions::to_string_t(relyingPartyUrl), config);
		web::http::http_request request(web::http::methods::POST);

		request.headers().add(L"Accept", L"application/json");
		request.headers().add(std::wstring(L"Authorization"), utility::conversions::to_utf16string(token));
		//request.headers().add(std::wstring(L"Signature"), utility::conversions::to_utf16string(signature));

		request.set_body(L"{\"account\":\"" + utility::conversions::to_string_t(account) + L"\",\"application\":\"" + utility::conversions::to_string_t(application) + L"\"}", L"application/json");

		return client.request(request).then([](web::http::http_response response)
		{
			if (response.status_code() != 200)
			{
				return response.extract_string(true).then([response](pplx::task<utility::string_t> task)
				{
					std::string body;
					try
					{
						body = utility::conversions::to_utf8string(task.get());
					}
					catch (const std::exception& ex)
					{
						body = std::string("<error retrieving the body: ") + ex.what() + ">";
					}
					return pplx::task_from_exception<web::json::value>(std::runtime_error("Relying party auth failed, code: " 
						+ std::to_string(response.status_code())
						+ ", reason: " + utility::conversions::to_utf8string(response.reason_phrase())
						+ ", body: " + body));
				});
			}
			return response.extract_json();
		})
			.then([](web::json::value token)
		{
			return utility::conversions::to_utf8string(token.as_string());
		});
	}
}
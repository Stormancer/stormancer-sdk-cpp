/***
* ==++==
*
* Copyright (c) Microsoft Corporation. All rights reserved.
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*
* ==--==
* =+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
*
* HTTP Library: Client-side APIs.
*
* This file contains a cross platform implementation based on Boost.ASIO.
*
* For the latest on this and related APIs, please see: https://github.com/Microsoft/cpprestsdk
*
* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
****/

#include "stdafx.h"

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-local-typedef"
#endif

#if defined(__clang__)
#pragma clang diagnostic pop
#endif




#include "cpprest/details/http_client_impl.h"
#include "cpprest/details/x509_cert_utilities.h"
#include <unordered_set>
#include <curl/curl.h>
#include "pplx/threadpool-std.h"




#include <openssl/err.h>
#include <openssl/ssl.h>
#include <openssl/crypto.h>


namespace
{


	void init_openssl_locks();
	std::unique_ptr<std::mutex[]> openssl_mutexes;

	void openssl_locking_function(int mode, int mutex_index, const char */*file*/, int /*line*/)
	{
		if (mode & CRYPTO_LOCK)
		{
			openssl_mutexes[mutex_index].lock();
		}
		else
		{
			openssl_mutexes[mutex_index].unlock();
		}
	}

	unsigned long openssl_threadid_function(void)
	{
		// The id returned by get_id() cannot be converted to int...
		// This might not be portable.
		std::hash<std::thread::id> hasher;
		return hasher(std::this_thread::get_id());
	}

	void init_openssl_locks()
	{
		// Create the number of mutextes required by OpenSSL
		openssl_mutexes = std::make_unique<std::mutex[]>(CRYPTO_num_locks());

		CRYPTO_set_locking_callback(openssl_locking_function);
		CRYPTO_set_id_callback(openssl_threadid_function);
	}



	// Curl initialization and cleanup should happen respectively at the very start and at the very end of the program.
	// The recommended way to achieve this in a third-party library like ours is to use a static object.
	// We do not need to worry about other parts of the program that might also use curl, as the library has an internal refcount.
	// See https://curl.haxx.se/libcurl/c/libcurl.html ("GLOBAL CONSTANTS" section)
	class cpprest_curl_initializer
	{
	public:
		cpprest_curl_initializer()
		{
			curl_global_init(CURL_GLOBAL_ALL);
			// TODO Figure out if this is needed on NX too. NX doesn't have CRYPTO_ functions

			init_openssl_locks();

		}

		~cpprest_curl_initializer()
		{
			curl_global_cleanup();
		}
	};

	cpprest_curl_initializer curl_static_initializer;
	}

namespace web {
	namespace http
	{
		namespace client
		{
			namespace details
			{
				enum class httpclient_errorcode_context
				{
					none = 0,
					connect,
					handshake,
					writeheader,
					writebody,
					readheader,
					readbody,
					close
				};

				class curl_http_client : public _http_client_communicator, public std::enable_shared_from_this < curl_http_client >
				{
				public: //Cons, dest
					curl_http_client(http::uri address, http_client_config client_config)
						: _http_client_communicator(std::move(address), std::move(client_config))
					{
					}

				public://Methods
					void send_request(const std::shared_ptr<request_context> &request_ctx) override;

					unsigned long open() override { return 0; }
				};

				//Additional data to track
				class curl_request_context : public request_context, public std::enable_shared_from_this < curl_request_context >
				{
					friend class curl_http_client;
				private:
					std::function<void(std::string)> logger()
					{
						return m_http_client->client_config().get_logger();
					}
				public:
					curl_request_context(const std::shared_ptr<_http_client_communicator> &client,
						http_request &request)
						: request_context(client, request)
						, m_request_sent(false)
						, curl(nullptr)
					{
						m_buffer = new uint8_t[m_http_client->client_config().chunksize()];

					}
					~curl_request_context()
					{
						if (curl != nullptr)
						{
							curl_easy_cleanup(curl);
						}
						if (_headers != nullptr)
						{
							curl_slist_free_all(_headers);
						}
						delete m_buffer;
					}

					static std::shared_ptr<request_context> create_request_context(std::shared_ptr<_http_client_communicator> &client, http_request &request)
					{
						return std::make_shared<curl_request_context>(client, request);
					}

					void start_request()
					{
						auto client = std::static_pointer_cast<curl_http_client>(m_http_client);
						auto uri = this->m_request.absolute_uri();
						//auto host = uri.host().c_str();
						//auto scheme = uri.scheme().c_str();
						//auto port = uri.port();
						curl = curl_easy_init();
						if (curl == nullptr)
						{
							this->report_error((unsigned long)httpclient_errorcode_context::connect, "Failed to create CURL client.");
							return;
						}

						curl_easy_setopt(curl, CURLOPT_URL, uri.to_string().c_str());
						auto method = this->m_request.method();
						curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, method.c_str());
						curl_easy_setopt(curl, CURLOPT_SSL_CTX_FUNCTION, ssl_ctx_callback);
						curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, true);

						m_content_length = m_request._get_impl()->_get_content_length();
						if (m_content_length > 0)
						{
							if (m_request.method() == http::methods::GET || m_request.method() == http::methods::HEAD)
							{
								this->report_exception(http_exception("A GET or HEAD request should not have an entity body."));
								return;
							}
							// TODO find out if we need more code to support chunked transfer encoding
							// Having it in the headers should be enough, otherwise maybe use CURLOPT_TRANSFER_ENCODING
						}

						AddRequestHeaders(this->m_request.headers());
						
						curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, my_trace);

						/* the DEBUGFUNCTION has no effect until we enable VERBOSE */
						curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

						/* example.com is redirected, so we tell libcurl to follow redirection */
						curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

						curl_easy_setopt(curl, CURLOPT_DEBUGDATA, this);
						curl_easy_setopt(curl, CURLOPT_HEADERDATA, this);
						curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);
						curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, read_header_callback);
						curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, read_response_callback);

						if (m_content_length > 0)
						{
							write_body();
						}
						else
						{
							write_without_body();
						}
					}

				private:

					

					void write_without_body()
					{
						const auto &progress = m_request._get_impl()->_progress_handler();
						if (progress)
						{
							try
							{
								(*progress)(message_direction::upload, m_uploaded);
							}
							catch (...)
							{
								report_exception(std::current_exception());
								return;
							}
						}







						this->logger()("sending web request without body: " + this->m_request.method() + ": " + this->m_request.absolute_uri().to_string());
						auto res = curl_easy_perform(curl);
						if (res != CURLE_OK)
						{
							auto error = curl_easy_strerror(res);
							this->report_exception(http_exception("Curl easy perform failed : " + std::string(error)));
							return;
						}
					}

					void write_body()
					{
						try
						{
							curl_easy_setopt(curl, CURLOPT_READDATA, m_buffer);
							/* provide the size of the upload */
							curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, m_content_length);

							/* send in the URL to store the upload as */

							/* upload please */
							curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
							curl_easy_setopt(curl, CURLOPT_READDATA, this);
							curl_easy_setopt(curl, CURLOPT_READFUNCTION, write_body_callback);







							this->logger()("sending web request with body: "+ this->m_request.method()+": "+ this->m_request.absolute_uri().to_string());
							auto res = curl_easy_perform(curl);
							if (res != CURLE_OK)
							{
								auto error = curl_easy_strerror(res);
								this->report_exception(http_exception("Curl easy perform failed : " + std::string(error)));
								return;
							}
							m_request_sent = true;
						}

						catch (...)
						{
							this->report_exception(std::current_exception());
							return;
						}
					}

					static size_t read_response_callback(char* ptr, size_t size, size_t n, void* userdata)
					{
						
						curl_request_context* that = (curl_request_context*)userdata;
						that->logger()("Received " + std::to_string(size) + "bytes for : " + that->m_request.method() + ": " + that->m_request.absolute_uri().to_string());

						if (that->m_downloaded == 0)
						{
							that->complete_headers();
						}

						const auto &progress = that->m_request._get_impl()->_progress_handler();
						if (progress)
						{
							try
							{
								(*progress)(message_direction::download, that->m_downloaded);
							}
							catch (...)
							{
								that->report_exception(std::current_exception());
								return 0;
							}
						}

						int actualRead = size * n;

						if (actualRead > 0)
						{
							auto writeBuffer = that->_get_writebuffer();
							try
							{
								that->m_downloaded += writeBuffer.putn_nocopy((const uint8_t*)ptr, actualRead).get();
							}
							catch (...)
							{
								that->report_exception(std::current_exception());
								return 0;
							}
						}

						if (that->m_content_length_response == that->m_downloaded)
						{
							that->complete_request(that->m_downloaded);
						}

						return actualRead;
					}

					static size_t write_body_callback(char *buffer, size_t size, size_t nitems, void *userdata)
					{
						curl_request_context* that = (curl_request_context*)userdata;

						auto readSize = std::min(that->m_content_length, size*nitems);

						const auto actualReadSize = that->m_request.body().streambuf().getn((uint8_t*)buffer, readSize).get();
						that->m_uploaded += static_cast<uint64_t>(actualReadSize);
						const auto &progress = that->m_request._get_impl()->_progress_handler();
						if (progress)
						{
							try
							{
								(*progress)(message_direction::upload, that->m_uploaded);
							}
							catch (...)
							{
								that->report_exception(std::current_exception());
								return 0;
							}
						}
						that->logger()("wrote "+std::to_string(readSize)+"bytes in body of : " + that->m_request.method() + ": " + that->m_request.absolute_uri().to_string());
						return readSize;
					}

					static size_t read_header_callback(char *buffer, size_t size, size_t nitems, void *userdata)
					{
						
						curl_request_context* that = (curl_request_context*)userdata;
						
						auto header = std::string(buffer, size * nitems);

						static std::string contentLengthField = "Content-Length:";
						if (std::memcmp(header.c_str(), contentLengthField.c_str(), contentLengthField.size()) == 0)
						{
							auto size = header.size() - contentLengthField.size();
							auto contentLengthStr = header.substr(contentLengthField.size(), size);
							that->m_content_length_response = std::stoi(contentLengthStr);
						}
						that->logger()("received header for web request: " + that->m_request.method() + ": " + that->m_request.absolute_uri().to_string() + "\n" + header);
						if (!that->_statusLineRead)
						{
							that->_statusLineRead = true;
							//Status line
							auto headers_stream = std::istringstream(header);

							std::string http_version;
							headers_stream >> http_version;
							status_code status_code;
							headers_stream >> status_code;

							std::string status_message;
							std::getline(headers_stream, status_message);
							if (status_code == 100) //Ignore status code 100 : Continue
							{
								that->_statusLineRead = false;
								
							}
							else
							{
								that->m_response.set_status_code(status_code);

								::web::http::details::trim_whitespace(status_message);
								that->m_response.set_reason_phrase(std::move(status_message));

								if (!headers_stream || http_version.substr(0, 5) != "HTTP/")
								{
									that->report_exception("Invalid HTTP status line");
									return 0;
								}
							}
						}

						//headers

						const auto colon = header.find(':');
						if (colon != std::string::npos)
						{
							auto name = header.substr(0, colon);
							auto value = header.substr(colon + 2, header.size() - (colon + 2));
							::web::http::details::trim_whitespace(name);
							::web::http::details::trim_whitespace(value);

							that->m_response.headers().add(std::move(name), std::move(value));
						}

						return size * nitems;
					}

					static CURLcode ssl_ctx_callback(CURL *curl, void *ssl_ctx, void *userptr)
					{
						const auto& certificates = http_client_config::root_certificates();





















						// Use raw OpenSSL everywhere else (linux)
						CURLcode rv = CURLE_ABORTED_BY_CALLBACK;
						for (const auto& cert : certificates)
						{
							BIO *cbio = BIO_new_mem_buf(cert.c_str(), cert.size());
							X509_STORE  *cts = SSL_CTX_get_cert_store((SSL_CTX *)ssl_ctx);
							X509_INFO *itmp;
							int i, count = 0;
							STACK_OF(X509_INFO) *inf;
							(void)curl;
							(void)userptr; // unused parameters

							if (!cts || !cbio) {
								return rv;
							}

							inf = PEM_X509_INFO_read_bio(cbio, NULL, NULL, NULL);

							if (!inf) {
								BIO_free(cbio);
								return rv;
							}

							for (i = 0; i < sk_X509_INFO_num(inf); i++) {
								itmp = sk_X509_INFO_value(inf, i);
								if (itmp->x509) {
									X509_STORE_add_cert(cts, itmp->x509);
									count++;
								}
								if (itmp->crl) {
									X509_STORE_add_crl(cts, itmp->crl);
									count++;
								}
							}

							sk_X509_INFO_pop_free(inf, X509_INFO_free);
							BIO_free(cbio);
						}

						rv = CURLE_OK;
						return rv;

					}
					static int my_trace(CURL*, curl_infotype type,
							char *data, size_t size,
							void *userp)
					{
						curl_request_context* that = (curl_request_context*)userp;
						
						
						std::string text;
						switch (type) {
						case CURLINFO_TEXT:

							text = "== Info: "+ std::string(data,size);
						default: /* in case a new one is introduced to shock us */
							return 0;

						case CURLINFO_HEADER_OUT:
							text = "=> Send header";
							break;
						case CURLINFO_DATA_OUT:
							text = "=> Send data";
							break;
						case CURLINFO_SSL_DATA_OUT:
							text = "=> Send SSL data";
							break;
						case CURLINFO_HEADER_IN:
							text = "<= Recv header";
							break;
						case CURLINFO_DATA_IN:
							text = "<= Recv data";
							break;
						case CURLINFO_SSL_DATA_IN:
							text = "<= Recv SSL data";
							break;
						}
						that->logger()("received debug log for web request: " + that->m_request.method() + ": " + that->m_request.absolute_uri().to_string() + " "+ text);
						
						return 0;
					}

					void AddRequestHeaders(web::http::http_headers headers)
					{

						for (auto iter = headers.begin(); iter != headers.end(); ++iter)
						{
							_headers = curl_slist_append(_headers, (iter->first + ":" + iter->second).c_str());
						}
						curl_easy_setopt(curl, CURLOPT_HTTPHEADER, _headers);
					}

				private:
					size_t m_content_length = 0;
					size_t m_content_length_response = 0;
					int m_uploaded = 0;
					int m_downloaded = 0;
					bool m_request_sent = false;
					uint8_t* m_buffer = nullptr;
					CURL* curl = nullptr;
					curl_slist *_headers = nullptr;
					bool _statusLineRead = false;
				};

				http_network_handler::http_network_handler(const uri &base_uri, const http_client_config &client_config)
					: m_http_client_impl(std::make_shared<curl_http_client>(base_uri, client_config))
				{
				}

				pplx::task<http_response> http_network_handler::propagate(http_request request)
				{
					auto context = details::curl_request_context::create_request_context(m_http_client_impl, request);

					// Use a task to externally signal the final result and completion of the task.
					auto result_task = pplx::create_task(context->m_request_completion);

					// Asynchronously send the response with the HTTP client implementation.
					m_http_client_impl->async_send_request(context);

					return result_task;
				}

				void curl_http_client::send_request(const std::shared_ptr<request_context> &request_ctx)
				{
					auto ctx = std::static_pointer_cast<curl_request_context>(request_ctx);

					try
					{
						//TODO : client_config().invoke_nativehandle_options(&(ctx->m_connection->m_socket));
						ctx->start_request();
					}
					catch (...)
					{
						request_ctx->report_exception(std::current_exception());
						return;
					}
				}
			}

			std::vector<std::string> http_client_config::m_root_certificates;
		}
	}

} // namespaces

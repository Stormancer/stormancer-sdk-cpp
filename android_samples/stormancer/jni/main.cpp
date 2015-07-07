#include <cpprest/http_client.h>
#include <cpprest/filestream.h>
#include <pplx/pplxtasks.h>

#include <RakPeerInterface.h>
#include <BitStream.h>
#include <PacketPriority.h>
#include <RakNetTypes.h>

#include "basic_bytebuf.h"
#include "basic_bytestream.h"

int main()
{
	auto peer = RakNet::RakPeerInterface::GetInstance();
	RakNet::RakPeerInterface::DestroyInstance(peer);

	std::string baseUri("http://www.google.com/");

	web::http::client::http_client client(baseUri);
	web::http::http_request request(web::http::methods::GET);

	client.request(request).then([](web::http::http_response response) {
		unsigned short statusCode = response.status_code();
		auto ss = new concurrency::streams::stringstreambuf;
		auto result = response.body().read_to_end(*ss).then([ss](size_t size) {
			std::string responseText = ss->collection();
			std::cout << responseText << std::endl;
		});
	});

	Stormancer::bytestream bs;
	bs << 1337;
	auto res = bs.str();

	return 0;
}

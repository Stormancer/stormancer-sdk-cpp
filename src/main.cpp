#include "stdafx.h"
#include <MsgPack.h>
#include <stormancer>

using namespace Stormancer;

int _tmain(int argc, _TCHAR* argv[])
{
	//testMsgPack();

	testClient();

	return 0;
}

void testClient()
{
	auto config = ClientConfiguration::forAccount("test", "echo");
	config.serverEndpoint = "http://localhost:8081";

	auto client = new Client(config);
	/*auto task = client.getPublicScene("test-scene", "hello").then([&](t) {
	auto scene = t.result;
	scene.addRoute("echo.out", [&](p) {
	cout << p.readObject<string>() << endl;
	});

	scene.connect().then([&](t2) {
	if (t2.isCompleted)
	{
	scene.send("echo.in", "hello");
	}
	else
	{
	cout << "Bad stuff happened..." << endl;
	}
	});
	});
	task.Wait();*/
}

void testMsgPack()
{
	std::stringbuf stream = std::stringbuf();
	MsgPack::Serializer serializer = MsgPack::Serializer(&stream);

	serializer << MsgPack__Factory(ArrayHeader(3)); //Next 3 elements belong in this array
	serializer << MsgPack::Factory(true);
	serializer << MsgPack__Factory(ArrayHeader(0));
	serializer << MsgPack::Factory("Hello World!");

	serializer << MsgPack__Factory(MapHeader(2));
	serializer << MsgPack::Factory("type");
	serializer << MsgPack::Factory("post");
	serializer << MsgPack::Factory("message");
	serializer << MsgPack::Factory("Hello World!");

	std::cout << "SERIALIZED: " << stream.str() << std::endl;

	//stream = std::stringbuf(stream.str());
	MsgPack::Deserializer deserializer = MsgPack::Deserializer(&stream);

	std::cout << "DESERIALIZED: ";

	std::unique_ptr<MsgPack::Element> element;

	deserializer >> element;

	std::cout << *element << std::endl;
	//std::cout << "type: " << (int)element->getType() << std::endl;
	//element->toJSON(std::cout);

	MsgPack::Array& array = dynamic_cast<MsgPack::Array&>(*element);
	for (int i = 0; i < array.getContainer()->size(); i++)
	{
		std::cout << *(array.getContainer()->at(i)) << std::endl;
	}

	MsgPack::Primitive& pri = dynamic_cast<MsgPack::Primitive&>(*(array.getContainer()->at(0)));
	std::cout << pri.getValue() << std::endl;

	MsgPack::Array& arr = dynamic_cast<MsgPack::Array&>(*(array.getContainer()->at(1)));
	std::cout << arr << std::endl;

	MsgPack::String& str = dynamic_cast<MsgPack::String&>(*(array.getContainer()->at(2)));
	std::cout << str.stdString() << std::endl;
}

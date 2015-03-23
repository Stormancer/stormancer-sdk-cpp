#include <stormancer.h>

//using namespace Stormancer;

void testMsgPack()
{
	/*std::basic_streambuf<unsigned char> stream;
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

	//cout << "SERIALIZED: " << stream.str() << endl;

	//stream = stringbuf(stream.str());
	MsgPack::Deserializer deserializer = MsgPack::Deserializer(&stream);

	cout << "DESERIALIZED: ";

	unique_ptr<MsgPack::Element> element;

	deserializer >> element;

	cout << *element << endl;
	//cout << "type: " << (int)element->getType() << endl;
	//element->toJSON(cout);

	MsgPack::Array& array = dynamic_cast<MsgPack::Array&>(*element);
	for (int i = 0; i < array.getContainer()->size(); i++)
	{
		cout << *(array.getContainer()->at(i)) << endl;
	}

	MsgPack::Primitive& pri = dynamic_cast<MsgPack::Primitive&>(*(array.getContainer()->at(0)));
	cout << pri.getValue() << endl;

	MsgPack::Array& arr = dynamic_cast<MsgPack::Array&>(*(array.getContainer()->at(1)));
	cout << arr << endl;

	MsgPack::String& str = dynamic_cast<MsgPack::String&>(*(array.getContainer()->at(2)));
	cout << str.stdString() << endl;*/
}

void testClient()
{
	//ClientConfiguration config("test", "echo");
	//config.serverEndpoint = "http://localhost:8081";

	//Client client(config);
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

int main(int argc, char* argv[])
{
	testMsgPack();

	//testClient();

	return 0;
}

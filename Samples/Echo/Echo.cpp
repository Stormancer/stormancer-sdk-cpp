#include <stormancer.h>

using namespace Stormancer;

void testClient();
void testMsgPack();

int main(int argc, char* argv[])
{
	testMsgPack();

	testClient();

	cin.get();

	return 0;
}

void testClient()
{
	auto config = new ClientConfiguration(L"test", L"echo");
	config->serverEndpoint = L"http://ipv4.fiddler:8081";

	Client client(config);
	auto task = client.getPublicScene(L"test-scene", L"hello").then([](pplx::task<Scene*> t) {
		auto scene = t.get();
		scene->addRoute(L"echo.out", [](Packet<IScenePeer>* p) {
			wstring str;
			p->serializer()->deserialize(p->stream, str);
			wcout << str << endl;
		});

		scene->connect().then([&scene](pplx::task<void> t2) {
			if (t2.is_done())
			{
				scene->sendPacket(L"echo.in", [](byteStream* stream) {
					*stream << L"hello";
				});
			}
			else
			{
				wcout << L"Bad stuff happened..." << endl;
			}
		});
	});

	try
	{
		task.wait();
	}
	catch (const exception &e)
	{
		wcout << L"Error exception:\n" << e.what();
	}

}

void testMsgPack()
{
	byteStream stream;
	MsgPack::Serializer serializer(stream.rdbuf());

	/*serializer << MsgPack__Factory(ArrayHeader(3)); //Next 3 elements belong in this array
	serializer << MsgPack::Factory(true);
	serializer << MsgPack__Factory(ArrayHeader(0));
	serializer << MsgPack::Factory("Hello World!");*/

	serializer << MsgPack__Factory(MapHeader(3));
	serializer << MsgPack::Factory("bool");
	serializer << MsgPack::Factory(true);
	serializer << MsgPack::Factory("int64");
	serializer << MsgPack::Factory((int64)1337);
	serializer << MsgPack::Factory("string");
	serializer << MsgPack::Factory("1337");

	string str = stream.str();

	cout << "SERIALIZED: " << str << endl;

	stream = byteStream(str);
	MsgPack::Deserializer deserializer(stream.rdbuf());

	cout << "DESERIALIZED: ";

	unique_ptr<MsgPack::Element> element;

	deserializer >> element;

	cout << *element << endl;
	cout << "type: " << (int)element->getType() << endl;
	element->toJSON(cout);

	MsgPack::Map& map = dynamic_cast<MsgPack::Map&>(*element);
	for (int i = 0; i < map.getContainer()->size(); i++)
	{
		cout << *(map.getContainer()->at(i)) << endl;
	}

	/*MsgPack::Primitive& pri = dynamic_cast<MsgPack::Primitive&>(*(map.getContainer()->at(0)));
	cout << pri.getValue() << endl;

	MsgPack::Array& arr = dynamic_cast<MsgPack::Array&>(*(map.getContainer()->at(1)));
	cout << arr << endl;

	MsgPack::String& str2 = dynamic_cast<MsgPack::String&>(*(map.getContainer()->at(2)));
	cout << str2.stdString() << endl;*/
}

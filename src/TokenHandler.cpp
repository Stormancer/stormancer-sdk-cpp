#include "stormancer.h"

namespace Stormancer
{
	TokenHandler::TokenHandler()
		: _tokenSerializer(new MsgPackSerializer())
	{
	}

	TokenHandler::~TokenHandler()
	{
	}

	void deserialize(bytestream* stream, ConnectionData& data)
	{
		/*bytestream stream2;
		MsgPack::Serializer serializer(stream2.rdbuf());

		//serializer << MsgPack__Factory(ArrayHeader(3)); //Next 3 elements belong in this array
		//serializer << MsgPack::Factory(true);
		//serializer << MsgPack__Factory(ArrayHeader(0));
		//serializer << MsgPack::Factory("Hello World!");

		serializer << MsgPack__Factory(MapHeader(3));
		serializer << MsgPack::Factory("bool");
		serializer << MsgPack::Factory(true);
		serializer << MsgPack::Factory("int64");
		serializer << MsgPack::Factory((int64)1337);
		serializer << MsgPack::Factory("string");
		serializer << MsgPack::Factory("1337");

		string str = stream2.str();

		wcout << "SERIALIZED: " << Helpers::to_wstring(str) << endl;

		stream2 = bytestream(str);
		MsgPack::Deserializer deserializer(stream2.rdbuf());

		wcout << "DESERIALIZED: ";

		unique_ptr<MsgPack::Element> element;

		deserializer >> element;

		//wcout << (*element) << endl;
		wcout << "type: " << (int)element->getType() << endl;
		//element->toJSON(wcout);

		MsgPack::Map& map = dynamic_cast<MsgPack::Map&>(*element);
		for (size_t i = 0; i < map.getContainer()->size(); i++)
		{
		//wcout << *(map.getContainer()->at(i)) << endl;
		}

		/*MsgPack::Primitive& pri = dynamic_cast<MsgPack::Primitive&>(*(map.getContainer()->at(0)));
		wcout << pri.getValue() << endl;

		MsgPack::Array& arr = dynamic_cast<MsgPack::Array&>(*(map.getContainer()->at(1)));
		wcout << arr << endl;

		MsgPack::String& str2 = dynamic_cast<MsgPack::String&>(*(map.getContainer()->at(2)));
		wcout << str2.stdString() << endl;*/

		////////////////////////////////////////////////////////////////

		bytestream stream3;
		string a = "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA";
		MsgPack::Serializer srlz(stream3.rdbuf());
		srlz << MsgPack::Factory(a);
		auto data2 = stream3.str();

		////////////////////////////////////////////////////////////////

		MsgPack::Deserializer dsrlz(stream->rdbuf());

		/*dsrlz.deserialize([](std::unique_ptr<MsgPack::Element> parsed) {
			stringstream ss;
			ss << (*parsed);
			string str = ss.str();
			cout << "Parsed: " << (*parsed) << "\n";
			parsed->toJSON(cout);
			return false;
		}, true);*/

		unique_ptr<MsgPack::Element> element;
		//unique_ptr<MsgPack::Element> element2;
		dsrlz >> element;
		//dsrlz >> element2;
		/*dsrlz >> element;
		dsrlz >> element;
		dsrlz >> element;
		dsrlz >> element;
		dsrlz >> element;
		dsrlz >> element;*/
		stringstream ss;
		ss << (*element);
		string elemstr = ss.str();
		//stringstream ss2;
		//ss2 << (*element2);
		//string elemstr2 = ss2.str();
		/*cout << "Parsed: " << (*element) << "\n";
		wcout << L"type: " << (int)element->getType() << endl;
		wcout << L"JSON: " << endl;*/

		/*deserialize(stream, data.Endpoints);
		deserialize(stream, data.AccountId);
		deserialize(stream, data.Application);
		deserialize(stream, data.SceneId);
		deserialize(stream, data.Routing);
		deserialize(stream, data.Issued);
		deserialize(stream, data.Expiration);
		deserialize(stream, data.UserData);
		deserialize(stream, data.ContentType);*/
	}

	SceneEndpoint* TokenHandler::decodeToken(wstring token)
	{
		token = Helpers::stringTrim(token, '"');
		wstring data = Helpers::stringSplit(token, L"-")[0];
		vector<byte> buffer = utility::conversions::from_base64(data);

		buffer = vector<byte>{0xD9, 36, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65};
		//buffer = vector<byte>{0xDA, 0x00, 36, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65};
		//buffer = vector<byte>{0xA9, 65, 99, 99, 111, 117, 110, 116, 73, 100};
		//buffer = vector<byte>{0x81, 0xA9, 65, 99, 99, 111, 117, 110, 116, 73, 100, 0xDA, 0x00, 36, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65};

		string buffer2 = Helpers::to_string(buffer);
		
		bytestream bs(buffer2);
		MsgPackSerializer* tknMsgPckSrlz = dynamic_cast<MsgPackSerializer*>(_tokenSerializer);
		if (tknMsgPckSrlz == nullptr)
		{
			throw exception("MsgPack serializer not found in TokenHandler::decodeToken");
		}

		auto result = new ConnectionData;
		//tknMsgPckSrlz->deserialize(bs, *result);
		deserialize(&bs, *result);

		auto sceneEp = new SceneEndpoint;
		sceneEp->token = token;
		sceneEp->tokenData = result;
		return sceneEp;
	}
};

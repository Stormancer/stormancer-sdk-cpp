#pragma once
#include "headers.h"
#include "Core/ISerializer.h"

namespace Stormancer
{
	class MsgPackSerializer : public ISerializer
	{
	public:
		MsgPackSerializer();
		~MsgPackSerializer();

#pragma region serialization

#pragma region Base template

		// Base template
		template<typename T>
		void serialize(T data, byteStream& stream)
		{
			MsgPack::Serializer srlz(&stream);
			srlz << MsgPack::Factory(data);
		}

#pragma endregion Base template
#pragma region Vector template

		// Array template
		template<typename T>
		void serialize(vector<T> data, byteStream& stream)
		{
			MsgPack::Serializer srlz(&stream);
			srlz << MsgPack__Factory(ArrayHeader(data.size()));
			for (int i = 0; i < data.size(); i++)
			{
				srlz << MsgPack::Factory(data[i]);
			}
		}

#pragma endregion Vector template
#pragma region Map template

		// Map template
		template<typename MT>
		void serialize(map<wstring, MT> data, byteStream& stream)
		{
			MsgPack::Serializer srlz(&stream);
			srlz << MsgPack__Factory(MapHeader(myMap.size()));
			for (auto it = data.begin(); it != data.end(); ++it)
			{
				srlz << MsgPack::Factory(it->first);
				//srlz << MsgPack::Factory(it->second);
				serialize<MT>(it->second, stream);
			}
		}

#pragma endregion Map template

#pragma endregion serialization

#pragma region deserialization

#pragma region Base template

		// Base template
		template<typename T>
		void deserialize(byteStream& stream, T& data)
		{
			MsgPack::Deserializer dsrlz(stream.rdbuf());
			unique_ptr<MsgPack::Element> element;
			dsrlz >> element;
			data = dynamic_cast<T&>(*element);
		}

		// Base template specializations

		// string
		template<>
		void deserialize<string>(byteStream& stream, string& str)
		{
			MsgPack::Deserializer dsrlz(stream.rdbuf());
			unique_ptr<MsgPack::Element> element;
			dsrlz >> element;
			str = dynamic_cast<MsgPack::String&>(*element).stdString();
		}

		// wstring
		template<>
		void deserialize<wstring>(byteStream& stream, wstring& str)
		{
			string strTmp;
			deserialize<string>(stream, strTmp);
			str = Helpers::to_wstring(strTmp);
		}

		// ConnectionData
		template<>
		void deserialize<ConnectionData>(byteStream& stream, ConnectionData& data)
		{
			MsgPack::Deserializer dsrlz(stream.rdbuf());

			dsrlz.deserialize([](std::unique_ptr<MsgPack::Element> parsed) {
				std::cout << "Parsed: " << *parsed << "\n";
				return false;
			}, true);

			/*unique_ptr<MsgPack::Element> element;
			dsrlz >> element;
			dsrlz >> element;
			dsrlz >> element;
			dsrlz >> element;
			dsrlz >> element;
			dsrlz >> element;
			dsrlz >> element;
			cout << "type: " << (int)element->getType() << endl;
			element->toJSON(cout);

			deserialize(stream, data.Endpoints);
			deserialize(stream, data.AccountId);
			deserialize(stream, data.Application);
			deserialize(stream, data.SceneId);
			deserialize(stream, data.Routing);
			deserialize(stream, data.Issued);
			deserialize(stream, data.Expiration);
			deserialize(stream, data.UserData);
			deserialize(stream, data.ContentType);*/
		}

#pragma endregion Base template
#pragma region Vector template

		// Array template
		template<typename VT>
		void deserialize(byteStream& stream, vector<VT>& v)
		{
			// TODO
		}

#pragma endregion Vector template
#pragma region Map template

		// Map template
		template<typename MT>
		void deserialize(byteStream& stream, map<wstring, MT>& m)
		{
			MsgPack::Deserializer dsrlz(&stream);
			unique_ptr<MsgPack::Element> element;
			dsrlz >> element;
			// TODO
		}

#pragma endregion Map template

#pragma endregion deserialization
	};
};

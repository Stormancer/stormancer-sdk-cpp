#pragma once
namespace Stormancer
{
	class Document
	{
	public:

		std::string id;
		std::string type;
		long long version;
		//Stored document
		std::string content;

		MSGPACK_DEFINE(id, type, version, content)
	};

	class PutResult
	{
	public:
		//true if the server could store/update the record, false in case of update conflict.
		bool success;
		//version of the document
		long long version;
		//currently stored document, if success == false
		std::string content;

		MSGPACK_DEFINE(success, version, content)
	};

	class SearchRequest
	{
	public:
		std::string Type;
		std::unordered_map < std::string, std::string > Must;
		int Skip = 0;
		int Size = 50;

		MSGPACK_DEFINE(Type, Must, Skip, Size)
	};

	class SearchResponse
	{
	public:
		std::vector<std::string> Hits;
		long Total;

		MSGPACK_DEFINE(Hits, Total)
	};
}
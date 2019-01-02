#pragma once

#include "stormancer/msgpack_define.h"
#include <string>

namespace Stormancer
{
	struct MetafileDto
	{
		std::string FileName;
		std::string URL;
		std::string Path;
		std::string MD5Hash;
		std::string MD5HashContent;

		MSGPACK_DEFINE(FileName, URL, Path, MD5Hash, MD5HashContent)
	};

	struct DocumentTest
	{
		std::string Type;
		std::string Content;

		MSGPACK_DEFINE(Type, Content)
	};
}

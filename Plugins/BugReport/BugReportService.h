#pragma once

#include "stormancer/headers.h"
#include "stormancer/Scene.h"
#include "stormancer/RPC/RpcService.h"
#include "stormancer/Serializer.h"

namespace Stormancer
{
	struct AttachedFileDescriptor
	{
		std::string name;
		std::string contentType;
		void* content;
		int contentLength;
	};

	struct BugReportAttachedFile
	{
		std::string name;
		std::string mimeType;
		int length;

		MSGPACK_DEFINE(name, mimeType, length);
	};

	struct BugReportMetadata
	{
		std::string category;
		std::string comment;
		std::vector<BugReportAttachedFile> files;

		MSGPACK_DEFINE(category, comment, files);
	};

	class BugReportService
	{
	public:

#pragma region public_methods

		BugReportService(Scene_ptr scene);

		//Reports a bug with optional attached files
		pplx::task<void> reportBug(std::string category, std::string comments, std::vector<AttachedFileDescriptor> files);

#pragma endregion

	private:

#pragma region private_members

		std::weak_ptr<Scene> _scene;
		std::shared_ptr<RpcService> _rpc;
		Serializer _serializer;

#pragma endregion
	};
}

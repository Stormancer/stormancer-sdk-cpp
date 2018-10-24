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

	struct PlayerReportAttachedFile
	{
		std::string name;
		std::string mimeType;
		int length;

		MSGPACK_DEFINE(name, mimeType, length);
	};

	struct PlayerReportMetadata
	{
		std::string type;
		std::string comment;
		std::string detailsJson;
		std::vector<PlayerReportAttachedFile> files;

		MSGPACK_DEFINE(type, comment, detailsJson, files);
	};

	class PlayerReportService
	{
	public:

#pragma region public_methods

		PlayerReportService(Scene_ptr scene);

		// Reports a player with optional attached files
		pplx::task<void> report(std::string type, std::string comments, std::string json, std::vector<AttachedFileDescriptor> files);

#pragma endregion

	private:

#pragma region private_members

		Scene_ptr _scene;
		std::shared_ptr<RpcService> _rpc;
		Serializer _serializer;

#pragma endregion
	};
}

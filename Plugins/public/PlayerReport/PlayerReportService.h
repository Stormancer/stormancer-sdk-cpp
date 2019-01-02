#pragma once


#include "stormancer/Scene.h"
#include "stormancer/RPC/Service.h"
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

	class PlayerReportService : public std::enable_shared_from_this<PlayerReportService>
	{
	public:

#pragma region public_methods

		PlayerReportService(std::shared_ptr<Scene> scene);

		// Reports a player with optional attached files
		pplx::task<void> report(std::string type, std::string comments, std::string json, std::vector<AttachedFileDescriptor> files);

#pragma endregion

	private:

#pragma region private_members

		std::shared_ptr<ILogger> _logger;
		std::weak_ptr<Scene> _scene;
		std::shared_ptr<RpcService> _rpcService;

		Serializer _serializer;

#pragma endregion
	};
}

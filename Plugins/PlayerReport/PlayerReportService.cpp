#include "Stormancer/stdafx.h"
#include "stormancer/headers.h"
#include "PlayerReportService.h"

namespace Stormancer
{
	PlayerReportService::PlayerReportService(Scene_ptr scene)
	{
		_scene = scene;
		_rpc = scene->dependencyResolver().lock()->resolve<RpcService>();
	}

	pplx::task<void> PlayerReportService::report(std::string type, std::string comments, std::string json, std::vector<AttachedFileDescriptor> files)
	{
		PlayerReportMetadata metadata;
		metadata.type = type;
		metadata.comment = comments;
		metadata.detailsJson = json;
		for (const auto& f : files)
		{
			auto attachedFile = PlayerReportAttachedFile();
			attachedFile.name = f.name;
			attachedFile.mimeType = f.contentType;
			attachedFile.length = f.contentLength;
			metadata.files.push_back(attachedFile);
		}

		return _rpc->rpcWriter("playerreporting.report", [&](obytestream* stream) {
			_serializer.serialize(stream, metadata);
			for (const auto& f : files)
			{
				stream->write((char*)f.content, f.contentLength);
			}
		});
	}
}
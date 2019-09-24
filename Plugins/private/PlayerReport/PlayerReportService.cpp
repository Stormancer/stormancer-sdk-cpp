#if defined(STORMANCER_CUSTOM_PCH)
#include STORMANCER_CUSTOM_PCH
#endif
#include "PlayerReport/PlayerReportService.h"


namespace Stormancer
{
	PlayerReportService::PlayerReportService(std::shared_ptr<Scene> scene)
	{
		_scene = scene;
		_rpcService = scene->dependencyResolver().resolve<RpcService>();
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

		auto& serializer = _serializer;
		return _rpcService->rpc("playerreporting.report", pplx::cancellation_token::none(), [serializer, &metadata, &files](obytestream& stream)
		{
			serializer.serialize(stream, metadata);
			for (const auto& f : files)
			{
				stream.write(reinterpret_cast<const byte*>(f.content), f.contentLength);
			}
		});
	}
}

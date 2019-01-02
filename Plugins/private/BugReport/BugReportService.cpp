#if defined(STORMANCER_CUSTOM_PCH)
#include STORMANCER_CUSTOM_PCH
#endif
#include "BugReport/BugReportService.h"

namespace Stormancer
{
	BugReportService::BugReportService(std::shared_ptr<Scene> scene)
	{
		_scene = scene;
		_rpc = scene->dependencyResolver()->resolve<RpcService>();
	}

	pplx::task<void> BugReportService::reportBug(std::string category, std::string comments, std::vector<AttachedFileDescriptor> files)
	{
		auto metadata = BugReportMetadata();
		metadata.category = category;
		metadata.comment = comments;
		for (const auto& f : files)
		{
			auto attachedFile = BugReportAttachedFile();
			attachedFile.name = f.name;
			attachedFile.mimeType = f.contentType;
			attachedFile.length = f.contentLength;
			metadata.files.push_back(attachedFile);
		}

		return _rpc->rpcWriter( "bugreporting.report", pplx::cancellation_token::none(), [&](obytestream* stream) {
			_serializer.serialize(stream, metadata);
			for (const auto& f : files)
			{
				stream->write((char*)f.content, f.contentLength);
			}
		});
	}
}
#include "BugReport/BugReportService.h"

namespace Stormancer
{
	BugReportService::BugReportService(std::shared_ptr<Scene> scene)
		: _scene(scene)
	{
		_rpc = scene->dependencyResolver().resolve<RpcService>();
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

		auto& serializer = _serializer;
		return _rpc->rpc("bugreporting.report", pplx::cancellation_token::none(), [&serializer, &metadata, &files](obytestream& stream)
		{
			serializer.serialize(stream, metadata);
			for (const auto& f : files)
			{
				stream.write((char*)f.content, f.contentLength);
			}
		});
	}
}
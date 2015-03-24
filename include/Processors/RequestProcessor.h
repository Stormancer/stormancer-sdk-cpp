#pragma once
#include "headers.h"
#include "IPacketProcessor.h"
#include "Core/ILogger.h"
#include "Infrastructure/Modules/IRequestModule.h"

namespace Stormancer
{
	class RequestProcessor : public IPacketProcessor
	{
	private:
		struct Request
		{
			time_t lastRefresh;
			uint16 id;
			Packet2 packet;
			pplx::task_completion_event<bool> tcs;
		};

	public:
		RequestProcessor(shared_ptr<ILogger*> logger, vector<shared_ptr<IRequestModule*>> modules);
		virtual ~RequestProcessor();

		void registerProcessor(PacketProcessorConfig config);
		void addSystemRequestHandler(byte msgId, function<pplx::task<void>(RequestContext)> handler);

	private:
		map<uint16, Request> _pendingRequests;
		shared_ptr<ILogger*> _logger;

		bool _isRegistered;
		map<byte, function<pplx::task<void>(RequestContext)>> _handlers;
	};
};

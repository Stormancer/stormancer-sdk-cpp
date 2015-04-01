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
			rx::observer<Packet<>> observer;
			pplx::task_completion_event<void> tcs;
			pplx::task<void> task = create_task(tcs);
		};

	public:
		RequestProcessor(shared_ptr<ILogger>& logger, vector<shared_ptr<IRequestModule>> modules);
		virtual ~RequestProcessor();

	public:
		void registerProcessor(PacketProcessorConfig& config);
		pplx::task<Packet<>> sendSystemRequest(shared_ptr<IConnection>& peer, byte msgId, function<void(byteStream&)> writer);
		pplx::task<Packet<>> sendSceneRequest(shared_ptr<IConnection> peer, byte sceneId, uint16 routeId, function<void(byteStream&)> writer);
		void addSystemRequestHandler(byte msgId, function<pplx::task<void>(RequestContext)>& handler);

	private:
		Request reserveRequestSlot(rx::observer<Packet<>> observer);

	private:
		map<uint16, Request> _pendingRequests;
		shared_ptr<ILogger> _logger;
		bool _isRegistered = false;
		map<byte, function<pplx::task<void>(RequestContext)>> _handlers;
	};
};

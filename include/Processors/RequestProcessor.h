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
			rx::observer<Packet<>*>* observer;
			pplx::task_completion_event<void> tcs;
			pplx::task<void> task = create_task(tcs);
		};

	public:
		RequestProcessor(ILogger* logger, vector<IRequestModule*> modules);
		virtual ~RequestProcessor();

	public:
		void registerProcessor(PacketProcessorConfig* config);
		pplx::task<Packet<>*> sendSystemRequest(IConnection* peer, byte msgId, function<void(bytestream*)> writer);
		pplx::task<Packet<>*> sendSceneRequest(IConnection* peer, byte sceneId, uint16 routeId, function<void(bytestream*)> writer);

	private:
		Request* reserveRequestSlot(rx::observer<Packet<>*>* observer);

	public:
		function<void(byte, function<pplx::task<void>(RequestContext*)>)> addSystemRequestHandler;

	private:
		map<uint16, Request*> _pendingRequests;
		ILogger* _logger;
		bool _isRegistered = false;
		map<byte, function<pplx::task<void>(RequestContext*)>> _handlers;
	};
};

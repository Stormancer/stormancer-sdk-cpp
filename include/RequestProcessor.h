#pragma once
#include "headers.h"
#include "IPacketProcessor.h"
#include "ILogger.h"
#include "IRequestModule.h"
#include "Request.h"

namespace Stormancer
{
	class RequestProcessor : public IPacketProcessor
	{
	public:
		RequestProcessor(ILogger* logger, vector<IRequestModule*> modules);
		virtual ~RequestProcessor();

	public:
		void registerProcessor(PacketProcessorConfig& config);
		pplx::task<Packet<>*> sendSystemRequest(IConnection* peer, byte msgId, function<void(bytestream*)> writer);

	private:
		shared_ptr<Request> reserveRequestSlot(PacketObserver&& observer);
		bool freeRequestSlot(uint16 requestId);

	public:
		function<void(byte, function<pplx::task<void>(RequestContext*)>)> addSystemRequestHandler;

	protected:
		map<uint16, shared_ptr<Request>> _pendingRequests;
		ILogger* _logger;
		bool _isRegistered = false;
		map<byte, function<pplx::task<void>(RequestContext*)>> _handlers;

	private:
		PacketProcessorConfig* _requestProcessorConfig;
	};
};

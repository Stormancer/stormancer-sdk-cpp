#pragma once
#include "headers.h"
#include "IPacketProcessor.h"
#include "ILogger.h"
#include "IRequestModule.h"

namespace Stormancer
{
	class RequestProcessor : public IPacketProcessor
	{
		typedef function<void(Packet<>*)> ObserverPacketHandler;
		typedef function<void(exception_ptr)> ObserverExceptionHandler;
		typedef rx::observer<Packet<>*, rx::static_observer<Packet<>*, ObserverPacketHandler, ObserverExceptionHandler, rxcpp::detail::OnCompletedEmpty>> PacketObserver;

	private:
		struct Request
		{
			time_t lastRefresh;
			uint16 id;
			PacketObserver* observer;
			pplx::task_completion_event<void> tcs;
			pplx::task<void> task = create_task(tcs);
		};

	public:
		RequestProcessor(ILogger* logger, vector<IRequestModule*> modules);
		virtual ~RequestProcessor();

	public:
		void registerProcessor(PacketProcessorConfig* config);
		pplx::task<Packet<>*> sendSystemRequest(IConnection* peer, byte msgId, function<void(bytestream*)> writer);

	private:
		Request* reserveRequestSlot(PacketObserver* observer);

	public:
		function<void(byte, function<pplx::task<void>(RequestContext*)>)> addSystemRequestHandler;

	protected:
		map<uint16, Request*> _pendingRequests;
		ILogger* _logger;
		bool _isRegistered = false;
		map<byte, function<pplx::task<void>(RequestContext*)>> _handlers;
	};
};

#pragma once
#include "headers.h"
#include "IPacketProcessor.h"
#include "ILogger.h"
#include "IRequestModule.h"

namespace Stormancer
{
	class RequestProcessor : public IPacketProcessor
	{
		using PacketObserver = rx::observer < Packet<>* > ;

	private:
		class Request
		{
		public:
			Request(PacketObserver&& observer);
			virtual ~Request();

		public:
			time_t lastRefresh = time(NULL);
			uint16 id = 0;
			PacketObserver observer;
			pplx::task_completion_event<void> tcs;
			pplx::task<void> task;
		};

	public:
		RequestProcessor(ILogger* logger, vector<IRequestModule*> modules);
		virtual ~RequestProcessor();

	public:
		void registerProcessor(PacketProcessorConfig* config);
		pplx::task<Packet<>*> sendSystemRequest(IConnection* peer, byte msgId, function<void(bytestream*)> writer);

	private:
		Request* reserveRequestSlot(PacketObserver&& observer);

	public:
		function<void(byte, function<pplx::task<void>(RequestContext*)>)> addSystemRequestHandler;

	protected:
		map<uint16, Request*> _pendingRequests;
		ILogger* _logger;
		bool _isRegistered = false;
		map<byte, function<pplx::task<void>(RequestContext*)>> _handlers;
	};
};

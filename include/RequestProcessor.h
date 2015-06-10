#pragma once
#include "headers.h"
#include "IPacketProcessor.h"
#include "ILogger.h"
#include "IRequestModule.h"
#include "Request.h"

namespace Stormancer
{
	/// Processes system requests.
	class RequestProcessor : public IPacketProcessor
	{
	public:
	
		/*! Constructor.
		\param logger The logger instance.
		\param modules A vector of modules.
		*/
		RequestProcessor(ILogger* logger, vector<IRequestModule*> modules);
		
		/// Destructor
		virtual ~RequestProcessor();

	public:
	
		/*! Registers the processor into the dispatcher.
		\param config The configuration of the processor.
		*/
		void registerProcessor(PacketProcessorConfig& config);
		
		/*! Sends a system request to the remote peer.
		\param peer A target peer.
		\param msgId The message id.
		\param writer A procedure writing the request parameters.
		\return An observable returning the request responses.
		*/
		pplx::task<shared_ptr<Packet<>>> sendSystemRequest(IConnection* peer, byte msgId, function<void(bytestream*)> writer);

	private:
	
		shared_ptr<Request> reserveRequestSlot(PacketObserver&& observer);
		
		bool freeRequestSlot(uint16 requestId);

	public:
	
		/*! Register a new system request handlers for the specified message Id.
		\param msgId The system message id.
		\param handler A function that handles message with the provided id.
		*/
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

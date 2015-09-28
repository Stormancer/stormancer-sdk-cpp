#pragma once
#include "headers.h"
#include "IPacketProcessor.h"
#include "ILogger.h"
#include "IRequestModule.h"
#include "SystemRequest.h"

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
		RequestProcessor(ILogger* logger, std::vector<IRequestModule*> modules);
		
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
		pplx::task<Packet_ptr> sendSystemRequest(IConnection* peer, byte msgId, std::function<void(bytestream*)> writer, PacketPriority priority = PacketPriority::MEDIUM_PRIORITY);

	private:
	
		SystemRequest_ptr reserveRequestSlot(pplx::task_completion_event<Packet_ptr> tce);
		
		SystemRequest_ptr freeRequestSlot(uint16 requestId);

	public:
	
		/*! Register a new system request handlers for the specified message Id.
		\param msgId The system message id.
		\param handler A function that handles message with the provided id.
		*/
		std::function<void(byte, std::function<pplx::task<void>(RequestContext*)>)> addSystemRequestHandler;

	protected:
		std::map<uint16, SystemRequest_ptr> _pendingRequests;
		std::mutex _mutexPendingRequests;
		ILogger* _logger;
		bool _isRegistered = false;
		std::map<byte, std::function<pplx::task<void>(RequestContext*)>> _handlers;

	private:
		PacketProcessorConfig* _requestProcessorConfig;
	};
};

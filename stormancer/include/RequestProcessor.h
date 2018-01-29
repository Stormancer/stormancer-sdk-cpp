#pragma once

#include "headers.h"
#include "IPacketProcessor.h"
#include "Logger/ILogger.h"
#include "IRequestModule.h"
#include "SystemRequest.h"
#include "PacketPriority.h"
#include "Serializer.h"

namespace Stormancer
{
	/// Processes system requests.
	class RequestProcessor : public IPacketProcessor
	{
	public:
		static void Initialize(std::shared_ptr<RequestProcessor> processor, std::vector<std::shared_ptr<IRequestModule>> modules);

		/// Constructor.
		/// \param logger The logger instance.
		/// \param modules A vector of modules.
		RequestProcessor(std::shared_ptr<ILogger> logger);

		/// Destructor
		virtual ~RequestProcessor();

		/// Registers the processor into the dispatcher.
		/// \param config The configuration of the processor.
		void registerProcessor(PacketProcessorConfig& config);

		/// Sends a system request to the remote peer.
		/// \param peer A target peer.
		/// \param msgId The message id.
		/// \param writer A procedure writing the request parameters.
		/// \return An observable returning the request responses.
		pplx::task<Packet_ptr> sendSystemRequest(IConnection* peer, byte msgId, const Writer& writer, PacketPriority priority = PacketPriority::MEDIUM_PRIORITY);

		/// Register a new system request handlers for the specified message Id.
		/// \param msgId The system message id.
		/// \param handler A function that handles message with the provided id.
		std::function<void(byte, std::function<pplx::task<void>(RequestContext*)>)> addSystemRequestHandler;

		template<typename T1, typename T2>
		pplx::task<T2> sendSystemRequest(IConnection* peer, byte id, const T1& parameter)
		{
			return sendSystemRequest(peer, id, [=, &parameter](obytestream* stream) {
				_serializer.serialize(stream, parameter);
			}).then([=](Packet_ptr packet) {
				return _serializer.deserializeOne<T2>(packet->stream);
			});
		}

	protected:

		std::map<uint16, SystemRequest_ptr> _pendingRequests;
		std::mutex _mutexPendingRequests;
		std::shared_ptr<ILogger> _logger;
		bool _isRegistered = false;
		std::map<byte, std::function<pplx::task<void>(RequestContext*)>> _handlers;
		Serializer _serializer;

	private:

		SystemRequest_ptr reserveRequestSlot(byte msgId,pplx::task_completion_event<Packet_ptr> tce);
		SystemRequest_ptr freeRequestSlot(uint16 requestId);
	};
};

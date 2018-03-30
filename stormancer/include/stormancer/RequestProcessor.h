#pragma once

#include "stormancer/headers.h"
#include "stormancer/IPacketProcessor.h"
#include "stormancer/Logger/ILogger.h"
#include "stormancer/IRequestModule.h"
#include "stormancer/SystemRequest.h"
#include "stormancer/PacketPriority.h"
#include "stormancer/Serializer.h"

namespace Stormancer
{
	/// Processes system requests.
	class RequestProcessor : public IPacketProcessor
	{
	public:

#pragma region public_methods

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
		pplx::task<Packet_ptr> sendSystemRequest(IConnection* peer, byte msgId, const Writer& writer, PacketPriority priority = PacketPriority::MEDIUM_PRIORITY, const pplx::cancellation_token& ct = pplx::cancellation_token::none());

		/// Register a new system request handlers for the specified message Id.
		/// \param msgId The system message id.
		/// \param handler A function that handles message with the provided id.
		std::function<void(byte, std::function<pplx::task<void>(RequestContext*)>)> addSystemRequestHandler;

		template<typename T1, typename T2>
		pplx::task<T1> sendSystemRequest(IConnection* peer, byte id, const T2& parameter, const pplx::cancellation_token& ct = pplx::cancellation_token::none())
		{
			return sendSystemRequest(peer, id, [=, &parameter](obytestream* stream) {
				_serializer.serialize(stream, parameter);
			}, PacketPriority::MEDIUM_PRIORITY, ct).then([=](Packet_ptr packet) {
				return _serializer.deserializeOne<T1>(packet->stream);
			}, ct);
		}

#pragma endregion

	private:

#pragma region private_methods

		SystemRequest_ptr reserveRequestSlot(byte msgId, pplx::task_completion_event<Packet_ptr> tce, const pplx::cancellation_token& ct = pplx::cancellation_token::none());

		SystemRequest_ptr freeRequestSlot(uint16 requestId);

#pragma endregion

#pragma region private_members

		std::map<uint16, SystemRequest_ptr> _pendingRequests;
		std::mutex _mutexPendingRequests;
		std::shared_ptr<ILogger> _logger;
		bool _isRegistered = false;
		std::map<byte, std::function<pplx::task<void>(RequestContext*)>> _handlers;
		Serializer _serializer;

#pragma endregion
	};
};

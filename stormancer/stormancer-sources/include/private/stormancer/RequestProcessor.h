#pragma once

#include "stormancer/BuildConfig.h"

#include "stormancer/IPacketProcessor.h"
#include "stormancer/Logger/ILogger.h"
#include "stormancer/IRequestModule.h"
#include "stormancer/SystemRequest.h"
#include "stormancer/PacketPriority.h"
#include "stormancer/Serializer.h"

namespace Stormancer
{
	/// Processes system requests.
	class RequestProcessor : public IPacketProcessor, public std::enable_shared_from_this<RequestProcessor>
	{
	public:

#pragma region public_methods

		static void Initialize(std::shared_ptr<RequestProcessor> processor, std::vector<std::shared_ptr<IRequestModule>> modules);

		/// Constructor.
		/// \param logger The logger instance.
		/// \param modules A vector of modules.
		RequestProcessor(std::shared_ptr<ILogger> logger);

		/// Destructor
		virtual ~RequestProcessor() = default;

		/// Registers the processor into the dispatcher.
		/// \param config The configuration of the processor.
		void registerProcessor(PacketProcessorConfig& config);

		/// Sends a system request to the remote peer.
		/// \param peer A target peer.
		/// \param msgId The message id.
		/// \param streamWriter A procedure writing the request parameters.
		/// \return An observable returning the request responses.
		pplx::task<Packet_ptr> sendSystemRequest(IConnection* peer, byte msgId, const StreamWriter& streamWriter, PacketPriority priority = PacketPriority::MEDIUM_PRIORITY, pplx::cancellation_token ct = pplx::cancellation_token::none());

		/// Register a new system request handlers for the specified message Id.
		/// \param msgId The system message id.
		/// \param handler A function that handles message with the provided id.
		void addSystemRequestHandler(byte msgId, std::function<pplx::task<void>(std::shared_ptr<RequestContext>)> handler);

		template<typename T1, typename T2>
		pplx::task<T1> sendSystemRequest(IConnection* peer, byte id, const T2& parameter, pplx::cancellation_token ct = pplx::cancellation_token::none())
		{
			auto serializer = _serializer;
			return sendSystemRequestInternal<T1>(peer, id, [serializer, &parameter](obytestream& stream)
			{
				serializer.serialize(stream, parameter);
			}, PacketPriority::MEDIUM_PRIORITY, ct);
		}

		template<typename T>
		pplx::task<T> sendSystemRequest(IConnection* peer, byte id, pplx::cancellation_token ct = pplx::cancellation_token::none())
		{
			return sendSystemRequestInternal<T>(peer, id, [](obytestream&) {}, PacketPriority::MEDIUM_PRIORITY, ct);
		}

		pplx::task<void> sendSystemRequest(IConnection* peer, byte id, pplx::cancellation_token ct = pplx::cancellation_token::none())
		{
			return sendSystemRequest(peer, id, [](obytestream&) {}, PacketPriority::MEDIUM_PRIORITY, ct)
				.then([](Packet_ptr)
			{
				// Packet is null for system requests that have no return value.
			}, ct);
		}

		template<typename T>
		pplx::task<void> sendSystemRequest(IConnection* peer, byte id, const T& parameter, pplx::cancellation_token ct = pplx::cancellation_token::none())
		{
			auto serializer = _serializer;
			return sendSystemRequest(peer, id, [serializer, &parameter](obytestream& stream)
			{
				serializer.serialize(stream, parameter);
			}, PacketPriority::MEDIUM_PRIORITY, ct)
				.then([](Packet_ptr)
			{
				// Packet is null for system requests that have no return value.
			}, ct);
		}

#pragma endregion

	private:

#pragma region private_methods

		SystemRequest_ptr reserveRequestSlot(byte msgId, pplx::cancellation_token ct = pplx::cancellation_token::none());

		SystemRequest_ptr freeRequestSlot(uint16 requestId);

		template<typename TResult>
		pplx::task<TResult> sendSystemRequestInternal(IConnection* peer, byte msgId, const StreamWriter& streamWriter, PacketPriority priority, pplx::cancellation_token ct)
		{
			std::string Tname = typeid(TResult).name();
			auto logger = _logger;
			auto serializer = _serializer;

			return sendSystemRequest(peer, msgId, streamWriter, priority, ct)
				.then([Tname, logger, serializer, msgId](Packet_ptr packet)
			{
				if (!packet)
				{
					auto msg = "Tried to return " + Tname + " from a system request of type " + std::to_string(msgId) + " that returned void.";
					logger->log(Stormancer::LogLevel::Error, "systemrequests", msg);
					throw std::runtime_error(msg.c_str());
				}
				else
				{
					return serializer.deserializeOne<TResult>(packet->stream);
				}
			}, ct);
		}

#pragma endregion

#pragma region private_members

		std::unordered_map<uint16, SystemRequest_ptr> _pendingRequests;
		std::mutex _mutexPendingRequests;
		std::shared_ptr<ILogger> _logger;
		bool _isRegistered = false;
		std::unordered_map<byte, std::function<pplx::task<void>(std::shared_ptr<RequestContext>)>> _handlers;
		Serializer _serializer;
		int systemRequestChannelUid = 0;

#pragma endregion
	};
}

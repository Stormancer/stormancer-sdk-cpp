#pragma once

#include "headers.h"
#include "Scene.h"
#include "RpcPlugin.h"
#include "Streams/bytestream.h"
#include "PacketPriority.h"

namespace Stormancer
{
	template<typename T = IScenePeer>
	class RpcRequestContext
	{
	public:

#pragma region public_methods

		RpcRequestContext(T* peer, Scene* scene, uint16 id, bool ordered, ibytestream* inputStream, pplx::cancellation_token token)
			: _scene(scene),
			_id(id),
			_ordered(ordered),
			_peer(peer),
			_inputStream(inputStream),
			_cancellationToken(token)
		{
		}

		~RpcRequestContext()
		{
		}

		T* remotePeer()
		{
			return _peer;
		}

		pplx::cancellation_token cancellationToken()
		{
			return _cancellationToken;
		}

		ibytestream* inputStream()
		{
			return _inputStream;
		}

		void sendValue(const Writer& writer, PacketPriority priority = PacketPriority::MEDIUM_PRIORITY)
		{
			if (!_scene)
			{
				throw std::runtime_error("The scene ptr is invalid");
			}

			_scene->sendPacket(RpcPlugin::nextRouteName, [=](obytestream* stream) {
				writeRequestId(stream);
				writer(stream);
			}, priority, (_ordered ? PacketReliability::RELIABLE_ORDERED : PacketReliability::RELIABLE), _rpcClientChannelIdentifier);
			_msgSent = 1;
		}

		void sendError(std::string errorMsg) const
		{
			if (!_scene)
			{
				throw std::runtime_error("The scene ptr is invalid");
			}

			_scene->sendPacket(RpcPlugin::errorRouteName, [=](obytestream* stream) {
				writeRequestId(stream);
				_serializer.serialize(stream, errorMsg);
			}, PacketPriority::MEDIUM_PRIORITY, PacketReliability::RELIABLE_ORDERED, _rpcClientChannelIdentifier);
		}

		void sendComplete() const
		{
			if (!_scene)
			{
				throw std::runtime_error("The scene ptr is invalid");
			}

			_scene->sendPacket(RpcPlugin::completeRouteName, [=](obytestream* stream) {
				(*stream) << _msgSent;
				writeRequestId(stream);
			}, PacketPriority::MEDIUM_PRIORITY, PacketReliability::RELIABLE_ORDERED, _rpcClientChannelIdentifier);
		}

#pragma endregion

	private:

#pragma region private_methods

		void writeRequestId(obytestream* stream) const
		{
			(*stream) << _id;
		}

#pragma endregion

#pragma region private_members

		T* _peer = nullptr;
		Scene* _scene = nullptr;
		ibytestream* _inputStream = nullptr;
		uint16 _id;
		bool _ordered;
		byte _msgSent;
		pplx::cancellation_token _cancellationToken;
		Serializer _serializer;
		const std::string _rpcClientChannelIdentifier = "RPC_client";

#pragma endregion
	};

	using RpcRequestContext_ptr = std::shared_ptr<RpcRequestContext<IScenePeer>>;
};

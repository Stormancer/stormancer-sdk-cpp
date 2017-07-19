#pragma once

#include "headers.h"
#include "Scene.h"
#include "RpcPlugin.h"
#include "bytestream.h"
#include "PacketPriority.h"

namespace Stormancer
{
	template<typename T = IScenePeer>
	class RpcRequestContext
	{
	public:
		RpcRequestContext(T* peer, Scene* scene, uint16 id, bool ordered, bytestream* inputStream, pplx::cancellation_token token)
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

	public:
		T* remotePeer()
		{
			return _peer;
		}

		pplx::cancellation_token cancellationToken()
		{
			return _cancellationToken;
		}

		bytestream* inputStream()
		{
			return _inputStream;
		}

		void sendValue(std::function<void(bytestream*)> writer, PacketPriority priority = PacketPriority::MEDIUM_PRIORITY)
		{
			if (!_scene)
			{
				throw std::runtime_error("The scene ptr is invalid");
			}

			_scene->sendPacket(RpcPlugin::nextRouteName, [this, writer](bytestream* bs) {
				writeRequestId(bs);
				writer(bs);
			}, priority, (_ordered ? PacketReliability::RELIABLE_ORDERED : PacketReliability::RELIABLE));
			_msgSent = 1;
		}

		void sendError(std::string errorMsg) const
		{
			if (!_scene)
			{
				throw std::runtime_error("The scene ptr is invalid");
			}

			_scene->sendPacket(RpcPlugin::errorRouteName, [this, errorMsg](bytestream* bs) {
				writeRequestId(bs);
				msgpack::pack(bs, errorMsg);
			}, PacketPriority::MEDIUM_PRIORITY, PacketReliability::RELIABLE_ORDERED);
		}

		void sendComplete() const
		{
			if (!_scene)
			{
				throw std::runtime_error("The scene ptr is invalid");
			}

			_scene->sendPacket(RpcPlugin::completeRouteName, [this](bytestream* bs) {
				*bs << _msgSent;
				writeRequestId(bs);
			}, PacketPriority::MEDIUM_PRIORITY, PacketReliability::RELIABLE_ORDERED);
		}

	private:
		void writeRequestId(bytestream* stream) const
		{
			*stream << _id;
		}

	private:
		T* _peer = nullptr;
		Scene* _scene = nullptr;
		bytestream* _inputStream = nullptr;
		uint16 _id;
		bool _ordered;
		byte _msgSent;
		pplx::cancellation_token _cancellationToken;
	};

	using RpcRequestContext_ptr = std::shared_ptr<RpcRequestContext<IScenePeer>>;
};

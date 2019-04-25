#pragma once

#include "stormancer/BuildConfig.h"

#include "stormancer/Scene.h"
#include "stormancer/Streams/bytestream.h"
#include "stormancer/PacketPriority.h"
#include "stormancer/IScenePeer.h"
#include "stormancer/Tasks.h"
#include "stormancer/Serializer.h"
#include "stormancer/RPC/Constants.h"

namespace Stormancer
{

	template<typename T = IScenePeer>
	class RpcRequestContext
	{
	public:

		friend class RpcService;

#pragma region public_methods

		RpcRequestContext(T* peer, std::weak_ptr<Scene> scene, uint16 id, bool ordered, ibytestream& inputStream, pplx::cancellation_token ct)
			: _peer(peer)
			, _scene(scene)
			, _inputStream(inputStream)
			, _id(id)
			, _ordered(ordered)
			, _cancellationToken(ct)
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

		ibytestream& inputStream()
		{
			return _inputStream;
		}

		void sendValue(const StreamWriter& streamWriter, PacketPriority priority = PacketPriority::MEDIUM_PRIORITY)
		{
			auto scene = _scene.lock();

			if (!scene)
			{
				throw std::runtime_error("The scene is invalid");
			}

			scene->send(Stormancer::rpc::nextRouteName, [this, streamWriter](obytestream& stream)
			{
				writeRequestId(stream);
				if (streamWriter)
				{
					streamWriter(stream);
				}
			}, priority, (_ordered ? PacketReliability::RELIABLE_ORDERED : PacketReliability::RELIABLE), _rpcClientChannelIdentifier);

			_msgSent = 1;
		}

		void sendError(std::string errorMsg) const
		{
			auto scene = _scene.lock();

			if (!scene)
			{
				throw std::runtime_error("The scene is invalid");
			}

			scene->send(Stormancer::rpc::errorRouteName, [this, errorMsg](obytestream& stream)
			{
				writeRequestId(stream);
				_serializer.serialize(stream, errorMsg);
			}, PacketPriority::MEDIUM_PRIORITY, PacketReliability::RELIABLE_ORDERED, _rpcClientChannelIdentifier);
		}

#pragma endregion

	private:

#pragma region private_methods

		void sendComplete() const
		{
			auto scene = _scene.lock();

			if (!scene)
			{
				throw std::runtime_error("The scene is invalid");
			}

			scene->send(Stormancer::rpc::completeRouteName, [this](obytestream& stream)
			{
				stream << _msgSent;
				writeRequestId(stream);
			}, PacketPriority::MEDIUM_PRIORITY, PacketReliability::RELIABLE_ORDERED, _rpcClientChannelIdentifier);
		}

		void writeRequestId(obytestream& stream) const
		{
			stream << _id;
		}

#pragma endregion

#pragma region private_members

		T* _peer = nullptr;
		std::weak_ptr<Scene> _scene;
		ibytestream& _inputStream;
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

#pragma once
#include "headers.h"

namespace Stormancer
{
	template<typename T>
	class RpcRequestContext
	{
	public:
		RpcRequestContext(T* peer, Scene* scene, uint16 id, bool ordered)
			: _peer(peer),
			_scene(scene),
			id(id),
			_ordered(ordered)
		{
		}

		virtual ~RpcRequestContext()
		{
		}

		T* remotePeer()
		{
			return _peer;
		}

		void sendValue(Action<bytestream&>& writer, PacketPriority priority)
		{
			_scene->sendPacket(RpcClientPlugin::nextRouteName, [this](bytestream& s) {
				this->writeRequestId(s);
				writer(s);
			}, priority, (_ordered ? PacketReliability::RELIABLE_ORDERED : PacketReliability::RELIABLE));
		}

		void sendError(std::string errorMsg)
		{
			_scene->sendPacket(RpcClientPlugin::errorRouteName, [this, errorMsg](bytestream& s) {
				this->writeRequestId(s);
				this->_peer->serializer().serialize(errorMsg, s);
			}, PacketPriority::MEDIUM_PRIORITY, PacketReliability::RELIABLE);
		}

		void sendComplete()
		{
			_scene->sendPacket(RpcClientPlugin::completedRouteName, [this](bytestream& s) {
				this->writeRequestId(s);
			}, PacketPriority::MEDIUM_PRIORITY, PacketReliability::RELIABLE);
		}

	private:
		void writeRequestId(bytestream& s)
		{
			s.write((char*)id, 2);
		}

	private:
		Scene* _scene;
		uint16 id;
		bool _ordered;
		T* _peer;
	};
};

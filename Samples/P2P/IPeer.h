#pragma once
#include <stormancer.h>

class IPeer
{
public:
	virtual void sendPacket(std::function<void(Stormancer::bytestream*)> writer, PacketPriority priority, PacketReliability reliability) = 0;
	virtual void onPacketReceived(std::function<void(void*)> callback) = 0;
	virtual void disconnect() = 0;
	virtual const char* userId() = 0;
	virtual Stormancer::ConnectionState connectionState() = 0;
	virtual Stormancer::Action<Stormancer::ConnectionState>::TIterator onConnectionStateChanged(std::function<void(Stormancer::ConnectionState)> callback) = 0;
	virtual Stormancer::Action<Stormancer::ConnectionState>& onConnectionStateChangedAction() = 0;
	virtual void destroy() = 0;

private:
	virtual void setConnectionState(Stormancer::ConnectionState connectionState) = 0;
};

#pragma once

#include "headers.h"
#include "IConnection.h"

namespace Stormancer
{
	class RelayConnection :public IConnection
	{
	public:

		RelayConnection(std::shared_ptr<IConnection> serverConnection, std::string address, uint64 id);
		virtual void sendSystem(byte msgId, std::function<void(bytestream*)> writer, PacketPriority priority = PacketPriority::MEDIUM_PRIORITY) override;
		virtual void sendToScene(byte sceneIndex, uint16 route, std::function<void(bytestream*)> writer, PacketPriority priority, PacketReliability reliability) override;
		virtual void sendRaw(byte msgId, std::function<void(bytestream*)> writer, PacketPriority priority, PacketReliability reliability) override;
		virtual void setApplication(std::string account, std::string application) override;
		virtual void close(std::string reason = "") override;
		virtual std::string ipAddress() const override;
		virtual int ping() const override;
		virtual uint64 id() const override;
		virtual time_t connectionDate() const override;
		virtual const std::string& account() const override;
		virtual const std::string& application() const override;
		virtual const std::map<std::string, std::string>& metadata() const override;
		virtual std::string metadata(const std::string& key) const override;
		virtual void setMetadata(const std::map<std::string, std::string>& metadata) override;
		virtual void setMetadata(const std::string& key, const std::string & value) override;
		virtual DependencyResolver* dependencyResolver() override;
		virtual ConnectionState getConnectionState() const override;
		virtual rxcpp::observable<ConnectionState> getConnectionStateChangedObservable() const override;
		virtual void setConnectionState(ConnectionState connectionState) override;

	private:
		std::shared_ptr<IConnection> _serverConnection;
		std::map<std::string, std::string> _metadata;
		uint64 _id;
		std::string _ipAddress;
		time_t _connectionDate = nowTime_t();
		std::shared_ptr<DependencyResolver> _dependencyResolver;
	};
}

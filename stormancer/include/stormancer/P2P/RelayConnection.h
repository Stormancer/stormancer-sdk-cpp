#pragma once

#include "stormancer/headers.h"
#include "stormancer/IConnection.h"

namespace Stormancer
{
	class RelayConnection : public IConnection
	{
	public:

		RelayConnection(std::shared_ptr<IConnection> serverConnection, std::string address, uint64 id);
		virtual void send(const Writer& writer, int channelUid, PacketPriority priority = PacketPriority::MEDIUM_PRIORITY, PacketReliability reliability = PacketReliability::RELIABLE_ORDERED, const TransformMetadata& transformMetadata = TransformMetadata()) override;
		void setApplication(std::string account, std::string application) override;
		void close(std::string reason = "") override;
		std::string ipAddress() const override;
		int ping() const override;
		uint64 id() const override;
		time_t connectionDate() const override;
		const std::string& account() const override;
		const std::string& application() const override;
		const std::map<std::string, std::string>& metadata() const override;
		std::string metadata(const std::string& key) const override;
		void setMetadata(const std::map<std::string, std::string>& metadata) override;
		void setMetadata(const std::string& key, const std::string & value) override;
		DependencyResolver* dependencyResolver() override;
		ConnectionState getConnectionState() const override;
		rxcpp::observable<ConnectionState> getConnectionStateChangedObservable() const override;
		void setConnectionState(ConnectionState connectionState) override;
		Action<std::string>::TIterator onClose(std::function<void(std::string)> callback) override;
		Action<std::string>& onCloseAction() override;

	private:
		std::shared_ptr<IConnection> _serverConnection;
		std::map<std::string, std::string> _metadata;
		uint64 _id;
		std::string _ipAddress;
		time_t _connectionDate = nowTime_t();
		std::shared_ptr<DependencyResolver> _dependencyResolver;
	};
}

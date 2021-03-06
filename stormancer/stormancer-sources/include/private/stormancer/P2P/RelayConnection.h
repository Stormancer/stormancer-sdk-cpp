#pragma once

#include "stormancer/BuildConfig.h"


#include "stormancer/IConnection.h"
#include "stormancer/Helpers.h"

namespace Stormancer
{
	class Serializer;
	class RelayConnection : public IConnection
	{
	public:

#pragma region public_methods

		RelayConnection(std::shared_ptr<IConnection> serverConnection, std::string address, uint64 remotePeerId, std::string p2pSessionId, std::weak_ptr<Serializer> serializer);
		virtual void send(const StreamWriter& streamWriter, int channelUid, PacketPriority priority = PacketPriority::MEDIUM_PRIORITY, PacketReliability reliability = PacketReliability::RELIABLE_ORDERED, const TransformMetadata& transformMetadata = TransformMetadata()) override;
		void setApplication(std::string account, std::string application) override;
		void close(std::string reason = "") override;
		std::string ipAddress() const override;
		int ping() const override;
		uint64 id() const override;
		std::string key() const override;
		time_t connectionDate() const override;
		const std::string& account() const override;
		const std::string& application() const override;
		const std::unordered_map<std::string, std::string>& metadata() const override;
		std::string metadata(const std::string& key) const override;
		void setMetadata(const std::unordered_map<std::string, std::string>& metadata) override;
		void setMetadata(const std::string& key, const std::string & value) override;
		const DependencyScope& dependencyResolver() const override;
		ConnectionState getConnectionState() const override;
		rxcpp::observable<ConnectionState> getConnectionStateChangedObservable() const override;
		void setConnectionState(ConnectionState connectionState) override;
		std::string sessionId() const override;

#pragma endregion

	protected:

#pragma region protected_methods

		void setSessionId(const std::string& sessionId) override;

#pragma endregion

	private:

#pragma region private_members

		std::shared_ptr<IConnection> _serverConnection;
		std::unordered_map<std::string, std::string> _metadata;
		std::string _p2pSessionId;
		uint64 _remotePeerId;
		std::string _ipAddress;
		time_t _connectionDate = nowTime_t();
		DependencyScope _dependencyResolver;
		std::weak_ptr<Serializer> _serializer;
		std::string _sessionId;

#pragma endregion
	};
}

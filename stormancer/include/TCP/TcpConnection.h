#pragma once

#include "headers.h"
#include "IConnection.h"
#include "SocketIncludes.h"
#include "SocketDefines.h"
#include "PacketPriority.h"

namespace Stormancer
{
	class TcpConnection : public IConnection
	{
		friend class TcpTransport;

	public:

#pragma region public_methods

		TcpConnection(std::shared_ptr<SOCKET> socketId, uint64 connectionId, std::string ip);
		~TcpConnection();
		void send(const Writer& writer, int channelUid, PacketPriority priority = PacketPriority::MEDIUM_PRIORITY, PacketReliability reliability = PacketReliability::RELIABLE_ORDERED, const TransformMetadata& transformMetadata = TransformMetadata()) override;
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
		void setMetadata(const std::string& key, const std::string& value) override;
		DependencyResolver* dependencyResolver() override;
		ConnectionState getConnectionState() const override;
		rxcpp::observable<ConnectionState> getConnectionStateChangedObservable() const override;
		Action<std::string>::TIterator onClose(std::function<void(std::string)> callback) override;
		Action<std::string>& onCloseAction() override;

#pragma endregion

	protected:

#pragma region protected_methods

		void setConnectionState(ConnectionState connectionState) override;

#pragma endregion

	private:

#pragma region private_methods

		void sendRaw(const Writer& writer, PacketPriority priority, PacketReliability reliability);

#pragma endregion

#pragma region private_members

		Action<std::string> _closeAction;
		std::weak_ptr<SOCKET> _socketId;
		uint64 _id;
		std::string _account;
		std::string _application;
		ConnectionState _connectionState = ConnectionState::Disconnected;
		std::string _ip;
		time_t _connectionDate = nowTime_t();
		std::map<std::string, std::string> _metadata;
		rxcpp::subjects::subject<ConnectionState> _connectionStateObservable;

#pragma endregion

		// Inherited via IConnection
		
	};
}

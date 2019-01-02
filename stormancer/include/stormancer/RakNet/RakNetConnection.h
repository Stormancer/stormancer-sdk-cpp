#pragma once

#include "RakPeerInterface.h"
#include "stormancer/headers.h"
#include "stormancer/IConnection.h"
#include "stormancer/PacketPriority.h"
#include "stormancer/Logger/ILogger.h"
#include "stormancer/Helpers.h"

namespace Stormancer
{
	/// Connection to the network using RakNet.
	class RakNetConnection : public IConnection
	{
	public:
		friend class ConnectionsRepository;
		friend class RakNetTransport;

#pragma region public_methods

		RakNetConnection(RakNet::RakNetGUID guid, int64 id, std::string key, std::weak_ptr<RakNet::RakPeerInterface> peer, ILogger_ptr logger, std::shared_ptr<DependencyResolver> resolver);
		~RakNetConnection();
		uint64 id() const override;
		std::string key() const override;

		time_t connectionDate() const override;
		const std::string& account() const override;
		const std::string& application() const override;
		ConnectionState getConnectionState() const override;
		RakNet::RakNetGUID guid() const;
		time_t lastActivityDate() const;
		std::string ipAddress() const override;
		bool operator==(RakNetConnection& other);
		bool operator!=(RakNetConnection& other);
		const std::map<std::string, std::string>& metadata() const override;
		std::string metadata(const std::string& key) const override;
		void setMetadata(const std::map<std::string, std::string>& metadata) override;
		void setMetadata(const std::string& key, const std::string& value) override;
		pplx::task<void> updatePeerMetadata(pplx::cancellation_token ct = pplx::cancellation_token::none()) override;
		std::shared_ptr<DependencyResolver> dependencyResolver() override;
		void close(std::string reason = "") override;
		virtual void send(const Writer& writer, int channelUid, PacketPriority priority = PacketPriority::MEDIUM_PRIORITY, PacketReliability reliability = PacketReliability::RELIABLE_ORDERED, const TransformMetadata& transformMetadata = TransformMetadata()) override;
		int ping() const override;
		void setApplication(std::string account, std::string application) override;
		
		rxcpp::observable<ConnectionState> getConnectionStateChangedObservable() const override;

		pplx::task<void> setTimeout(std::chrono::milliseconds timeout, pplx::cancellation_token ct = pplx::cancellation_token::none()) override;

		template<typename T>
		void registerComponent(T* component)
		{
			_localData[typeid(T).hash_code()] = static_cast<void*>(component);
		}

		template<typename T>
		bool getComponent(T* component = nullptr)
		{
			size_t hash_code = typeid(T).hash_code();
			if (mapContains(_localData, hash_code))
			{
				if (component != nullptr)
				{
					component = static_cast<T*>(_localData[hash_code]);
				}
				return true;
			}

			component = nullptr;
			return false;
		}

#pragma endregion

	protected:

#pragma region protected_methods

		void setConnectionState(ConnectionState connectionState) override;

#pragma endregion

	private:

#pragma region private_members

		std::map<std::string, std::string> _metadata;
		std::string _account;
		std::string _application;
		uint64 _id = 0;
		std::string _key;
		time_t _connectionDate = nowTime_t();
		std::weak_ptr<RakNet::RakPeerInterface> _peer;
		RakNet::RakNetGUID _guid;
		time_t _lastActivityDate = nowTime_t();
		std::map<size_t, void*> _localData;
		std::shared_ptr<DependencyResolver> _dependencyResolver;
		ConnectionState _connectionState = ConnectionState::Disconnected;
		rxcpp::subjects::subject<ConnectionState> _connectionStateObservable;
		
		ILogger_ptr _logger;
		std::string _closeReason;
		
#pragma endregion
	};
};

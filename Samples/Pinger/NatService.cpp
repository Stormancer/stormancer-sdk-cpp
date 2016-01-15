#include "NatService.h"
#include "RakNetPeer.h"

NatService::NatService(Stormancer::Client* client)
	: _client(client)
{
	_rakPeerInterface = client->dependencyResolver()->resolve<RakNet::RakPeerInterface>();
	_natPunchthroughClient = new RakNet::NatPunchthroughClient;
	_natPunchthroughClient->SetDebugInterface(this);
	_rakPeerInterface->AttachPlugin(_natPunchthroughClient);
}

NatService::~NatService()
{
}

pplx::task<Stormancer::Result<Stormancer::stringMap>*> NatService::getP2PData(const char* userId2)
{
	pplx::task_completion_event<Stormancer::Result<Stormancer::stringMap>*> tce;
	auto result = new Stormancer::Result<Stormancer::stringMap>();

	std::string userId = userId2;

	Stormancer::ILogger::instance()->log(Stormancer::LogLevel::Info, "NatService::getP2PData", "getP2PData for", userId2);

	_uploadTask.then([this, userId, tce, result]() {
		auto observable = _rpcService->rpc("nat.getp2pdata", [this, userId](Stormancer::bytestream* stream) {
			msgpack::pack(stream, "userId");
			msgpack::pack(stream, userId);
		}, PacketPriority::MEDIUM_PRIORITY);

		auto onNext = [this, tce, result](Stormancer::Packetisp_ptr packet) {
			Stormancer::ILogger::instance()->log(Stormancer::LogLevel::Info, "NatService::getP2PData", "getP2PData next", "");

			std::string buffer;
			*packet->stream >> buffer;

			msgpack::unpacked upckd;
			msgpack::unpack(upckd, buffer.data(), buffer.size());
			Stormancer::stringMap p2pData;
			upckd.get().convert(&p2pData);

			result->set(p2pData);
			tce.set(result);
		};

		auto onError = [tce, result, observable](const char* error) {
			Stormancer::ILogger::instance()->log(Stormancer::LogLevel::Error, "NatService::getP2PData", "getP2PData error", error);

			result->setError(1, error);
			tce.set(result);

			observable->destroy();
		};

		auto onComplete = [observable]() {
			Stormancer::ILogger::instance()->log(Stormancer::LogLevel::Info, "NatService::getP2PData", "getP2PData complete", "");

			observable->destroy();
		};

		observable->subscribe(onNext, onError, onComplete);
	});

	return pplx::create_task(tce);
}

pplx::task<Stormancer::Result<RakNetPeer*>*> NatService::openNat(const char* userId2, bool autoConnect)
{
	pplx::task_completion_event<Stormancer::Result<RakNetPeer*>*> tce;

	std::string userId = userId2;

	auto t = getP2PData(userId.c_str());
	t.then([this, userId, tce, autoConnect](Stormancer::Result<Stormancer::stringMap>* r) {
		if (r->success())
		{
			auto p2pData = r->get();
			if (Stormancer::mapContains(p2pData, std::string("raknet")))
			{
				auto rakNetGUID = RakNet::RakNetGUID(std::stoull(p2pData["raknet"]));
				auto serverSystemAddress = RakNet::SystemAddress(_transport->host(), _transport->port());
				auto rakNetPeer = new RakNetPeer(rakNetGUID, _rakPeerInterface, userId, autoConnect);
				rakNetPeer->_tce = tce;
				rakNetPeer->setConnectionState(Stormancer::ConnectionState::Connecting);
				_peers[rakNetGUID] = rakNetPeer;
				rakNetPeer->onConnectionStateChanged([this, rakNetPeer](Stormancer::ConnectionState state) {
					if (state == Stormancer::ConnectionState::Disconnected)
					{
						_peers.erase(rakNetPeer->rakNetGUID());
					}
				});
				auto openNatResult = _natPunchthroughClient->OpenNAT(rakNetGUID, serverSystemAddress);
				if (!openNatResult)
				{
					Stormancer::ILogger::instance()->log(Stormancer::LogLevel::Warn, "NatService::openNat", "Can't open Nat", "RakNet OpenNat returned 'false'");
				}
			}
		}
		else
		{
			auto result = new Stormancer::Result<RakNetPeer*>();
			result->setError(1, r->reason());
			tce.set(result);
		}
		delete r;
	});

	return pplx::create_task(tce);
}

pplx::task<Stormancer::Result<std::vector<RakNetPeer*>>*> NatService::openNatGroup(std::vector<std::string> userIds, bool autoConnect)
{
	pplx::task_completion_event<Stormancer::Result<std::vector<RakNetPeer*>>*> tce;

	std::vector<pplx::task<Stormancer::Result<RakNetPeer*>*>> tasks;

	for (auto userId : userIds)
	{
		tasks.push_back(openNat(userId.c_str(), autoConnect));
	}

	auto task = pplx::when_all(std::begin(tasks), std::end(tasks)).then([this, userIds, tasks, tce](std::vector<Stormancer::Result<RakNetPeer*>*> rakNetGUIDs) {
		std::vector<RakNetPeer*> res;
		std::string error;
		bool success = true;
		for (auto i = 0; i < tasks.size(); i++)
		{
			auto t = tasks[i];
			auto userId = userIds[i];
			auto r = t.get();
			if (r->success())
			{
				auto rakNetPeer = r->get();
				res.push_back(rakNetPeer);
			}
			else
			{
				success = false;
				error += "\n";
				error += r->reason();
			}
			delete r;
		}

		auto result = new Stormancer::Result<std::vector<RakNetPeer*>>();
		if (success)
		{
			result->set(res);
		}
		else
		{
			result->setError(1, error.c_str());
		}
		tce.set(result);
	});

	return pplx::create_task(tce);
}

RakNet::RakPeerInterface* NatService::getRakPeer() const
{
	return _rakPeerInterface;
}

void NatService::upload()
{
	if (_scene)
	{
		if (!_rpcService)
		{
			_rpcService = _scene->dependencyResolver()->resolve<Stormancer::IRpcService>();
		}

		pplx::task_completion_event<void> tce;

		auto observable = _rpcService->rpc("nat.updatep2pdata", [this](Stormancer::bytestream* stream) {
			Stormancer::stringMap p2pData;
			p2pData["raknet"] = _rakPeerInterface->GetGuidFromSystemAddress(RakNet::UNASSIGNED_SYSTEM_ADDRESS).ToString();
			msgpack::pack(stream, p2pData);
		}, PacketPriority::MEDIUM_PRIORITY);

		auto onNext = [this](Stormancer::Packetisp_ptr packet) {
			Stormancer::ILogger::instance()->log(Stormancer::LogLevel::Info, "NatService::upload", "Nat upload next", "");
		};

		auto onError = [](const char* error) {
			Stormancer::ILogger::instance()->log(Stormancer::LogLevel::Error, "NatService::upload", "Nat upload error", error);
		};

		auto onComplete = [tce]() {
			Stormancer::ILogger::instance()->log(Stormancer::LogLevel::Info, "NatService::upload", "Nat upload complete", "");
			tce.set();
		};

		observable->subscribe(onNext, onError, onComplete);

		_uploadTask = pplx::create_task(tce);
	}
}

void NatService::transportEvent(Stormancer::byte* ID, RakNet::Packet* packet, RakNet::RakPeerInterface* rakPeerInterface)
{
	myRakNetGUID = rakPeerInterface->GetGuidFromSystemAddress(RakNet::UNASSIGNED_SYSTEM_ADDRESS);

	if (!_rakPeerInterface)
	{
		_rakPeerInterface = rakPeerInterface;
	}

	auto rakNetGUID = packet->guid;

	switch (*ID)
	{
	case DefaultMessageIDTypes::ID_CONNECTION_REQUEST_ACCEPTED:
	case DefaultMessageIDTypes::ID_NEW_INCOMING_CONNECTION:
	{
		Stormancer::ILogger::instance()->log(Stormancer::LogLevel::Trace, "NatService::transportEvent", "New connection", packet->systemAddress.ToString(true, ':'));
		auto rakNetPeer = _peers[rakNetGUID];
		rakNetPeer->setConnectionState(Stormancer::ConnectionState::Connected);
		if (_onNewP2PConnection)
		{
			_onNewP2PConnection(rakNetPeer);
		}
		break;
	}
	case DefaultMessageIDTypes::ID_NO_FREE_INCOMING_CONNECTIONS:
	case DefaultMessageIDTypes::ID_CONNECTION_ATTEMPT_FAILED:
	{
		Stormancer::ILogger::instance()->log(Stormancer::LogLevel::Trace, "NatService::transportEvent", "Connection failed", packet->systemAddress.ToString(true, ':'));
		auto rakNetPeer = _peers[rakNetGUID];
		rakNetPeer->setConnectionState(Stormancer::ConnectionState::Disconnected);
		break;
	}
	case DefaultMessageIDTypes::ID_DISCONNECTION_NOTIFICATION:
	case DefaultMessageIDTypes::ID_CONNECTION_LOST:
	{
		Stormancer::ILogger::instance()->log(Stormancer::LogLevel::Trace, "NatService::transportEvent", "Peer lost the connection.", packet->systemAddress.ToString(true, ':'));
		auto rakNetPeer = _peers[rakNetGUID];
		rakNetPeer->setConnectionState(Stormancer::ConnectionState::Disconnected);
		break;
	}
	case DefaultMessageIDTypes::ID_NAT_PUNCHTHROUGH_SUCCEEDED:
	{
		Stormancer::ILogger::instance()->log(Stormancer::LogLevel::Info, "NatService::transportEvent", "Open NAT succeed", "");
		RakNetPeer* rakNetPeer = nullptr;
		if (Stormancer::mapContains(_peers, rakNetGUID))
		{
			rakNetPeer = _peers[rakNetGUID];
		}
		else
		{
			rakNetPeer = new RakNetPeer(rakNetGUID, _rakPeerInterface, "", false);
			_peers[rakNetGUID] = rakNetPeer;
		}
		rakNetPeer->setSystemAddress(packet->systemAddress);
		rakNetPeer->_weAreSender = (packet->data[1] ? true : false);
		if (rakNetPeer->weAreSender())
		{
			auto result = new Stormancer::Result<RakNetPeer*>();
			result->set(rakNetPeer);
			rakNetPeer->_tce.set(result);
		}
		if (rakNetPeer->autoConnect())
		{
			auto msg = std::string() + rakNetGUID.ToString() + " is connecting to " + rakNetPeer->systemAddress().ToString();
			Stormancer::ILogger::instance()->log(Stormancer::LogLevel::Trace, "NatService::transportEvent", msg.c_str(), "");
			rakNetPeer->connect();
		}
		break;
	}
	case DefaultMessageIDTypes::ID_NAT_PUNCHTHROUGH_FAILED:
	case DefaultMessageIDTypes::ID_NAT_TARGET_NOT_CONNECTED:
	case DefaultMessageIDTypes::ID_NAT_TARGET_UNRESPONSIVE:
	case DefaultMessageIDTypes::ID_NAT_CONNECTION_TO_TARGET_LOST:
	case DefaultMessageIDTypes::ID_NAT_ALREADY_IN_PROGRESS:
	{
		std::string error = "RakNet Message ID: " + std::to_string(*ID);
		Stormancer::ILogger::instance()->log(Stormancer::LogLevel::Info, "NatService::transportEvent", "Open NAT failed", error.c_str());
		if (Stormancer::mapContains(_peers, rakNetGUID))
		{
			auto rakNetPeer = _peers[rakNetGUID];
			rakNetPeer->setConnectionState(Stormancer::ConnectionState::Disconnected);
			auto result = new Stormancer::Result<RakNetPeer*>();
			result->setError(1, "Open Nat failed");
			rakNetPeer->_tce.set(result);
			delete rakNetPeer;
		}
		break;
	}
	default:
	{
		auto idstr = std::to_string(*ID);
		Stormancer::ILogger::instance()->log(Stormancer::LogLevel::Debug, "NatService::transportEvent", "ID", idstr.c_str());
		if (*ID >= DefaultMessageIDTypes::ID_USER_PACKET_ENUM)
		{
			//auto idstr = std::to_string(*ID);
			//Stormancer::ILogger::instance()->log(Stormancer::LogLevel::Debug, "NAT", "Custom ID", idstr.c_str());
			auto peer = _peers[rakNetGUID];
			if (peer->_onPacketReceived)
			{
				peer->_onPacketReceived((void*)packet);
			}
		}
		else
		{
			//auto idstr = std::to_string(*ID);
			//Stormancer::ILogger::instance()->log(Stormancer::LogLevel::Debug, "NAT", "Not custom ID", idstr.c_str());
		}
		break;
	}
	}
}

void NatService::OnClientMessage(const char* msg)
{
	Stormancer::ILogger::instance()->log(Stormancer::LogLevel::Info, "NatService::OnClientMessage", "NAT RakNet message received", msg);
}

void NatService::onNewP2PConnection(std::function<void(RakNetPeer*)> callback)
{
	_onNewP2PConnection = callback;
}

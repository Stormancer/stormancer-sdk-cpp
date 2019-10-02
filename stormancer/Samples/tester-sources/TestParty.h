#pragma once
#include "test.h"
#include "Party/Party.hpp"
#include "Users/Users.hpp"
#include "GameFinder/GameFinderPlugin.h"
#include "stormancer/Utilities/TaskUtilities.h"
#include "TestHelpers.h"

#include <sstream>

class TestParty
{
public:
	static void runTests(const Stormancer::Tester& tester)
	{
		testCreatePartyBadRequest(tester);
		testPartyEventHandler(tester);
		testPartyFull(tester);
	}

private:

	class TestPartyEventHandlerFailure : public Stormancer::Party::IPartyEventHandler
	{
	public:
		class Plugin : public Stormancer::IPlugin
		{
			void registerClientDependencies(Stormancer::ContainerBuilder& builder)
			{
				builder.registerDependency<TestPartyEventHandlerFailure>().as<Stormancer::Party::IPartyEventHandler>().asSelf().singleInstance();
			}
		};

		class TestException {};

	private:
		pplx::task<void> onJoiningParty(std::shared_ptr<Stormancer::Party::PartyApi> party, std::string scene) override
		{
			throw TestException();
		}

		pplx::task<void> onLeavingParty(std::shared_ptr<Stormancer::Party::PartyApi> party, std::string scene) override
		{
			_leavingTce.set();
			return pplx::task_from_result();
		}

		pplx::task_completion_event<void> _leavingTce;

	public:
		pplx::task<void> awaitOnLeaving()
		{
			return TestHelpers::taskFailAfterTimeout(pplx::create_task(_leavingTce), "onLeavingParty() should have been called", std::chrono::seconds(2));
		}
	};

	static pplx::task<std::shared_ptr<Stormancer::IClient>> makeClient(const Stormancer::Tester& tester, Stormancer::IPlugin* additionalPlugin = nullptr)
	{
		using namespace Stormancer;

		auto conf = Configuration::create(tester.endpoint(), tester.account(), tester.application());
		conf->logger = tester.logger();
		conf->addPlugin(new Users::UsersPlugin);
		conf->addPlugin(new GameFinderPlugin);
		conf->addPlugin(new Party::PartyPlugin);
		if (additionalPlugin)
		{
			conf->addPlugin(additionalPlugin);
		}

		auto client = IClient::create(conf);
		auto users = client->dependencyResolver().resolve<Users::UsersApi>();
		users->getCredentialsCallback = []
		{
			Users::AuthParameters params;
			params.type = "test";
			params.parameters["testkey"] = "testvalue";
			return pplx::task_from_result<Users::AuthParameters>(params);
		};
		return users->login().then([client] { return client; });
	}

	static pplx::task<std::vector<std::shared_ptr<Stormancer::IClient>>> makeClients(const Stormancer::Tester& tester, int count)
	{
		std::vector<pplx::task<std::shared_ptr<Stormancer::IClient>>> clientTasks;
		for (int i = 0; i < count; i++)
		{
			clientTasks.push_back(makeClient(tester));
		}

		return pplx::when_all(clientTasks.begin(), clientTasks.end())
			.then([clientTasks](pplx::task<std::vector<std::shared_ptr<Stormancer::IClient>>> task)
		{
			try
			{
				return task.get();
			}
			catch (...)
			{
				handleExceptions(clientTasks.begin(), clientTasks.end());
				throw;
			}
		});
	}

	static void testCreatePartyBadRequest(const Stormancer::Tester& tester)
	{
		using namespace Stormancer;
		tester.logger()->log(LogLevel::Info, "TestParty", "testCreatePartyBadRequest");

		auto client = makeClient(tester).get();

		auto party = client->dependencyResolver().resolve<Party::PartyApi>();
		assertex(!party->isInParty(), "I should not be in a party initially");
		
		Party::PartyRequestDto request = {};
		try
		{
			party->createParty(request).get();
		}
		catch (const std::exception& ex)
		{
			tester.logger()->log(LogLevel::Trace, "testCreatePartyBadRequest", "expected createParty error", ex.what());
			tester.logger()->log(LogLevel::Info, "TestParty", "testCreatePartyBadRequest PASSED");
			return;
		}
		throw std::runtime_error("createParty should have failed because of bad request");
	}

	static void testPartyEventHandler(const Stormancer::Tester& tester)
	{
		using namespace Stormancer;
		using namespace TestHelpers;

		tester.logger()->log(LogLevel::Info, "TestParty", "testPartyEventHandler");

		auto client = makeClient(tester, new TestPartyEventHandlerFailure::Plugin).get();
		auto party = client->dependencyResolver().resolve<Party::PartyApi>();
		auto eventHandler = client->dependencyResolver().resolve<TestPartyEventHandlerFailure>();

		Party::PartyRequestDto request = {};
		request.platformSessionId = ""; // This will make a guid
		request.GameFinderName = "testGameFinder";
		request.CustomData = "customData";

		bool joinedTriggered = false;
		auto joinedSub = party->subscribeOnJoinedParty([&joinedTriggered]
		{
			joinedTriggered = true;
		});
		bool leftTriggered = false;
		auto leftSub = party->subscribeOnLeftParty([&leftTriggered](Party::MemberDisconnectionReason)
		{
			leftTriggered = true;
		});

		try
		{
			party->createParty(request).get();
			throw std::runtime_error("createParty should have failed because of event handler throwing in onJoiningParty");
		}
		catch (const TestPartyEventHandlerFailure::TestException&) { /* OK */ }
		catch (const std::exception& ex)
		{
			throw std::runtime_error("createParty unexpectedly failed: " + std::string(ex.what()));
		}

		if (joinedTriggered)
		{
			throw std::runtime_error("OnJoinedParty event should not have been triggered");
		}

		eventHandler->awaitOnLeaving().get();

		if (leftTriggered)
		{
			throw std::runtime_error("OnLeftParty should not have been triggered");
		}

		tester.logger()->log(LogLevel::Info, "TestParty", "testPartyEventHandler PASSED");
	}

	static void testPartyFull(const Stormancer::Tester& tester)
	{
		using namespace Stormancer;
		using namespace TestHelpers;

		int numClients = 10;
		tester.logger()->log(LogLevel::Info, "TestParty", "Connecting with "+std::to_string(numClients)+" clients...");
		auto clients = makeClients(tester, numClients).get();
		tester.logger()->log(LogLevel::Info, "TestParty", "Done");

		tester.logger()->log(LogLevel::Info, "TestParty", "testCreateParty");
		testCreateParty(clients[0]).get();
		tester.logger()->log(LogLevel::Info, "TestParty", "testCreateParty PASSED");

		tester.logger()->log(LogLevel::Info, "TestParty", "testInvitations");
		testInvitations(clients[0], clients.begin()+1, clients.end(), tester).get();
		tester.logger()->log(LogLevel::Info, "TestParty", "testInvitations PASSED");

		tester.logger()->log(LogLevel::Info, "TestParty", "testMembersUpdate");
		testMembersUpdate(clients).get();
		tester.logger()->log(LogLevel::Info, "TestParty", "testMembersUpdate PASSED");

		tester.logger()->log(LogLevel::Info, "TestParty", "testUpdatePartySettings");
		testUpdatePartySettings(clients).get();
		tester.logger()->log(LogLevel::Info, "TestParty", "testUpdatePartySettings PASSED");

		tester.logger()->log(LogLevel::Info, "TestParty", "testFindGame");
		tester.logger()->log(LogLevel::Info, "TestParty::testFindGame", "Creating "+ std::to_string(numClients)+" more clients...");
		auto clients2 = makeClients(tester, numClients).get();
		tester.logger()->log(LogLevel::Info, "TestParty::testFindGame", "Putting them all into a party...");
		testCreateParty(clients2[0]).get();
		testInvitations(clients2[0], clients2.begin() + 1, clients2.end(), tester).get();
		tester.logger()->log(LogLevel::Info, "TestParty::testFindGame", "Running a GameFinder request for both parties...");
		testFindGame(clients, clients2).get();
		tester.logger()->log(LogLevel::Info, "TestParty", "testFindGame PASSED");
	}

	static pplx::task<void> testCreateParty(std::shared_ptr<Stormancer::IClient> client)
	{
		using namespace Stormancer;
		using namespace TestHelpers;

		auto party = client->dependencyResolver().resolve<Party::PartyApi>();
		auto users = client->dependencyResolver().resolve<Users::UsersApi>();

		Party::PartyRequestDto request = {};
		request.platformSessionId = ""; // This will make a guid
		request.GameFinderName = "testGameFinder";
		request.CustomData = "customData";

		pplx::task_completion_event<void> joinedSubTce;
		auto subJoined = party->subscribeOnJoinedParty([joinedSubTce]
		{
			joinedSubTce.set();
		});
		SelfObservingTask<void> joinedSubTask = pplx::create_task(joinedSubTce).then([subJoined] {});
		SelfObservingTask<void> checkSettingsTask = testSettingsValidityOnPartyJoined(party, users, request);

		return party->createParty(request).then([=]
		{
			assertex(party->isInParty(), "I should be in a party after a successful createParty");
			assertex(party->getPendingInvitations().empty(), "Pending invitations should be empty");

			failAfterTimeout(joinedSubTce, "OnJoinedParty subscription was not triggered in time");
			SelfObservingTask<void> checkMembersTask = testMembersValidityOnPartyJoined(client);

			auto creationCheckTasks = { *joinedSubTask, *checkSettingsTask, *checkMembersTask };
			return pplx::when_all(creationCheckTasks.begin(), creationCheckTasks.end());
		});
	}

	static pplx::task<void> testSettingsValidityOnPartyJoined(std::shared_ptr<Stormancer::Party::PartyApi> party,
		std::shared_ptr<Stormancer::Users::UsersApi> users,
		const Stormancer::Party::PartyRequestDto& request
	)
	{
		using namespace Stormancer;
		using namespace TestHelpers;

		pplx::task_completion_event<void> tce;
		auto sub = party->subscribeOnUpdatedPartySettings([tce, party, request, users](Party::PartySettings)
		{
			assertex(party->getPartySettings().customData == request.CustomData, "CustomData should be the same as in the request", tce);
			assertex(party->getPartySettings().gameFinderName == request.GameFinderName, "gameFinderName should be the same as in the request", tce);
			assertex(party->getPartyLeaderId() == users->userId(), "I should be the leader ; actual leader value from settings: " + party->getPartyLeaderId(), tce);
			assertex(party->isLeader(), "PartyApi should also report that I'm the leader", tce);
			tce.set();
		});

		failAfterTimeout(tce, "PartySettings update was not received in time", std::chrono::seconds(10));
		return pplx::create_task(tce).then([sub] {});
	}

	static pplx::task<void> testMembersValidityOnPartyJoined(std::shared_ptr<Stormancer::IClient> client)
	{
		using namespace Stormancer;

		auto party = client->dependencyResolver().resolve<Party::PartyApi>();
		auto users = client->dependencyResolver().resolve<Users::UsersApi>();

		Party::PartyUserDto expectedMember(users->userId());
		expectedMember.isLeader = true;
		expectedMember.partyUserStatus = Party::PartyUserStatus::NotReady;

		return awaitMembersConsistency(client, { expectedMember });
	}

	static std::string memberToString(const Stormancer::Party::PartyUserDto& member)
	{
		using namespace Stormancer;

		std::stringstream ss;
		ss << "{\n";
		ss << "\tuserId: " << member.userId << "\n";
		ss << "\tisReady: " << (member.partyUserStatus == Party::PartyUserStatus::Ready ? "true" : "false") << "\n";
		ss << "\tisLeader: " << member.isLeader << "\n";
		ss << "\tuserData: " << member.userData << "\n";
		ss << "}";

		return ss.str();
	}

	static std::string membersToString(const std::vector<Stormancer::Party::PartyUserDto>& members)
	{
		using namespace Stormancer;

		std::stringstream ss;
		for (const auto& member : members)
		{
			ss << memberToString(member) << "\n";
		}

		return ss.str();
	}

	static pplx::task<void> awaitMembersConsistency(std::shared_ptr<Stormancer::IClient> client,
		const std::vector<Stormancer::Party::PartyUserDto>& expectedMembers,
		std::chrono::milliseconds timeout = std::chrono::seconds(3))
	{
		using namespace Stormancer;
		using namespace TestHelpers;

		pplx::task_completion_event<void> tce;
		auto party = client->dependencyResolver().resolve<Party::PartyApi>();
		auto sub = party->subscribeOnUpdatedPartyMembers([tce, expectedMembers](std::vector<Party::PartyUserDto> members)
		{
			if (membersAreEqual(members, expectedMembers))
			{
				tce.set();
			}
		});

		if (party->isInParty() && membersAreEqual(party->getPartyMembers(), expectedMembers))
		{
			tce.set();
			sub->unsubscribe();
		}

		failAfterTimeout(tce, [party, expectedMembers]
		{
			std::stringstream ss;
			ss << "Members are not consistent with what was expected.\n";
			ss << "Expected Members:\n";
			ss << membersToString(expectedMembers);
			ss << "Actual Members:\n";
			if (party->isInParty())
			{
				ss << membersToString(party->getPartyMembers());
			}
			else
			{
				ss << "(not connected to party)\n";
			}
			return ss.str();
		}, timeout);

		return pplx::create_task(tce).then([sub] {});
	}

	static pplx::task<void> awaitMembersConsistency(
		const std::vector<std::shared_ptr<Stormancer::IClient>>& clients,
		const std::vector<Stormancer::Party::PartyUserDto>& expectedMembers,
		std::chrono::milliseconds timeout = std::chrono::seconds(3)
	)
	{
		std::vector<pplx::task<void>> tasks;
		tasks.reserve(clients.size());
		std::transform(clients.begin(), clients.end(), std::back_inserter(tasks), [&](std::shared_ptr<Stormancer::IClient> client)
		{
			return awaitMembersConsistency(client, expectedMembers, timeout);
		});

		return when_all_handle_exceptions(tasks);
	}

	static std::string settingsToString(const Stormancer::Party::PartySettings& settings)
	{
		std::stringstream ss;
		ss << "gameFinderName: " << settings.gameFinderName << "\n";
		ss << "customData: " << settings.customData << "\n";
		return ss.str();
	}

	static pplx::task<void> awaitSettingsConsistency(
		std::shared_ptr<Stormancer::IClient> client,
		const Stormancer::Party::PartySettings& settings,
		std::chrono::milliseconds timeout = std::chrono::seconds(3)
	)
	{
		using namespace Stormancer;
		using namespace TestHelpers;

		pplx::task_completion_event<void> tce;
		auto party = client->dependencyResolver().resolve<Party::PartyApi>();
		auto sub = party->subscribeOnUpdatedPartySettings([tce, settings](Party::PartySettings newSettings)
		{
			if (newSettings.gameFinderName == settings.gameFinderName && newSettings.customData == settings.customData)
			{
				tce.set();
			}
		});

		if (party->isInParty() && party->getPartySettings().gameFinderName == settings.gameFinderName && party->getPartySettings().customData == settings.customData)
		{
			tce.set();
			sub->unsubscribe();
		}

		failAfterTimeout(tce, [party, settings]
		{
			std::stringstream ss;
			ss << "Settings are not consistent with what was expected.\n";
			ss << "Expected Settings:\n";
			ss << settingsToString(settings);
			ss << "Actual Settings:\n";
			if (party->isInParty())
			{
				ss << settingsToString(party->getPartySettings());
			}
			else
			{
				ss << "(not connected to party)\n";
			}
			return ss.str();
		}, timeout);

		return pplx::create_task(tce).then([sub] {});
	}

	static pplx::task<void> awaitSettingsConsistency(
		const std::vector<std::shared_ptr<Stormancer::IClient>>& clients,
		const Stormancer::Party::PartySettings& settings,
		std::chrono::milliseconds timeout = std::chrono::seconds(3)
	)
	{
		std::vector<pplx::task<void>> tasks;
		tasks.reserve(clients.size());
		std::transform(clients.begin(), clients.end(), std::back_inserter(tasks), [&](std::shared_ptr<Stormancer::IClient> client)
		{
			return awaitSettingsConsistency(client, settings, timeout);
		});

		return when_all_handle_exceptions(tasks);
	}

	static bool membersAreEqual(const std::vector<Stormancer::Party::PartyUserDto>& members, const std::vector<Stormancer::Party::PartyUserDto>& expectedMembers)
	{
		if (members.size() != expectedMembers.size())
		{
			return false;
		}

		for (const auto& expectedMember : expectedMembers)
		{
			auto memberIt = std::find_if(members.begin(), members.end(), [&expectedMember](const auto& member) { return expectedMember.userId == member.userId; });
			if (memberIt == members.end())
			{
				return false;
			}
			if (
				memberIt->isLeader != expectedMember.isLeader ||
				memberIt->partyUserStatus != expectedMember.partyUserStatus ||
				memberIt->userData != expectedMember.userData
				)
			{
				return false;
			}
		}

		return true;
	}

	static pplx::task<void> testInvitation(std::shared_ptr<Stormancer::IClient> sender, std::shared_ptr<Stormancer::IClient> recipient)
	{
		using namespace Stormancer;
		using namespace TestHelpers;

		auto senderUsers = sender->dependencyResolver().resolve<Users::UsersApi>();
		auto senderId = senderUsers->userId();
		auto party = sender->dependencyResolver().resolve<Party::PartyApi>();
		auto party2 = recipient->dependencyResolver().resolve<Party::PartyApi>();

		pplx::task_completion_event<void> inviteReceivedTce;
		auto party2inviteSub = party2->subscribeOnInvitationReceived([inviteReceivedTce, senderId](Party::PartyInvitation invite)
		{
			if (invite.UserId != senderId)
			{
				inviteReceivedTce.set_exception(std::runtime_error("Bad sender Id: " + invite.UserId + ", should be " + senderId));
			}
			else
			{
				inviteReceivedTce.set();
			}
		});

		auto recipientId = recipient->dependencyResolver().resolve<Users::UsersApi>()->userId();
		auto inviteTask = party->invitePlayer(recipientId);

		failAfterTimeout(inviteReceivedTce, "invite was not received on time by user "+recipientId);
		return pplx::create_task(inviteReceivedTce).then([=]
		{
			party2inviteSub->unsubscribe();
			auto invites = party2->getPendingInvitations();
			assertex(invites.size() == 1, "User " + recipientId + " should have only 1 invite");

			recipient->dependencyResolver().resolve<ILogger>()->log(LogLevel::Debug, "testInvitation", "User connecting to party", recipientId);
			return taskFailAfterTimeout(party2->joinParty(invites[0]), "JoinParty did not complete in time for user "+recipientId, std::chrono::seconds(10));
		})
			.then([=](pplx::task<void> task)
		{
			// debug help
			auto sender2 = sender;
			auto recpt = recipient;
			try
			{
				task.get();
				return taskFailAfterTimeout(inviteTask, "Invitation task should have completed for "+senderId+" after invitee ("+recipientId+") joined the party");
			}
			catch (...)
			{
				inviteTask.then([](pplx::task<void> t) { try { t.get(); } catch (...) {} });
				throw;
			}
		});
	}

	static Stormancer::Party::PartyUserDto makeUserDto(
		std::shared_ptr<Stormancer::IClient> user,
		bool isLeader = false,
		Stormancer::Party::PartyUserStatus status = Stormancer::Party::PartyUserStatus::NotReady,
		std::string userData = ""
	)
	{
		using namespace Stormancer;

		Party::PartyUserDto dto(user->dependencyResolver().resolve<Users::UsersApi>()->userId());
		dto.isLeader = isLeader;
		dto.partyUserStatus = status;
		dto.userData = userData;

		return dto;
	}

	template <typename It>
	static pplx::task<void> testInvitations(std::shared_ptr<Stormancer::IClient> sender, It recipientsFirst, It recipientsEnd, const Stormancer::Tester& tester)
	{
		using namespace Stormancer;

		std::vector<Party::PartyUserDto> users;
		std::vector<pplx::task<void>> tasks;

		users.push_back(makeUserDto(sender, true));
		// Need a separate loop for users vector since it needs to be full when passing it to awaitMembersConsistency()
		for (auto client = recipientsFirst; client != recipientsEnd; client++)
		{
			users.push_back(makeUserDto(*client));
		}

		auto logger = tester.logger();
		for (auto client = recipientsFirst; client != recipientsEnd; client++)
		{
			auto clientId = (*client)->dependencyResolver().template resolve<Users::UsersApi>()->userId();
			tasks.push_back(testInvitation(sender, *client).then([logger, clientId]
			{
				logger->log(LogLevel::Debug, "testInvitations", "Invite complete", clientId);
			}));
			tasks.push_back(awaitMembersConsistency(*client, users, std::chrono::seconds(15)).then([logger, clientId]
			{
				logger->log(LogLevel::Debug, "testInvitations", "MemberConsistency complete recipient", clientId);
			}));
		}
		auto senderId = sender->dependencyResolver().resolve<Users::UsersApi>()->userId();
		tasks.push_back(awaitMembersConsistency(sender, users, std::chrono::seconds(15)).then([logger, senderId]
		{
			logger->log(LogLevel::Debug, "testInvitations", "MemberConsistency complete sender", senderId);
		}));

		return pplx::when_all(tasks.begin(), tasks.end())
			.then([tasks](pplx::task<void> task)
		{
			try
			{
				task.get();
			}
			catch (...)
			{
				handleExceptions(tasks.begin(), tasks.end());
				throw;
			}
		});
	}

	static int getLeader(const std::vector<std::shared_ptr<Stormancer::IClient>>& clients)
	{
		using namespace Stormancer;

		for (int i=0; i<clients.size(); i++)
		{
			if (getParty(clients[i])->isLeader())
			{
				return i;
			}
		}

		return -1;
	}

	static std::shared_ptr<Stormancer::Party::PartyApi> getParty(const std::shared_ptr<Stormancer::IClient>& client)
	{
		return client->dependencyResolver().resolve<Stormancer::Party::PartyApi>();
	}
	
	static pplx::task<void> testMembersUpdate(const std::vector<std::shared_ptr<Stormancer::IClient>>& clients)
	{
		using namespace Stormancer;
		using namespace TestHelpers;

		std::vector<pplx::task<void>> tasks;
		std::vector<Party::PartyUserDto> members;
		for (int i=0; i<clients.size(); i++)
		{
			std::string data = "client" + std::to_string(i) + "data";
			Party::PartyUserStatus status = (i % 2 == 0) ? Party::PartyUserStatus::Ready : Party::PartyUserStatus::NotReady;
			auto party = getParty(clients[i]);

			tasks.push_back(party->updatePlayerData(data));
			tasks.push_back(party->updatePlayerStatus(status));
			members.push_back(makeUserDto(clients[i], party->isLeader(), status, data));
		}
		tasks.push_back(awaitMembersConsistency(clients, members, std::chrono::seconds(10)));

		return when_all_handle_exceptions(tasks);
	}

	static pplx::task<void> testUpdatePartySettings(const std::vector<std::shared_ptr<Stormancer::IClient>>& clients)
	{
		using namespace Stormancer;
		using namespace TestHelpers;

		int leader = getLeader(clients);
		assertex(leader != -1, "Party should have a leader");

		Party::PartySettings settings = getParty(clients[leader])->getPartySettings();
		settings.customData = "newCustomData";
		int nextLeader = (leader + 1) % clients.size();

		std::vector<pplx::task<void>> tasks;
		std::vector<Party::PartyUserDto> members;
		for (int i = 0; i < clients.size(); i++)
		{
			auto party = getParty(clients[i]);
			if (i == leader)
			{
				auto nextLeaderId = clients[nextLeader]->dependencyResolver().resolve<Users::UsersApi>()->userId();
				tasks.push_back(party->updatePartySettings(settings).then([party, nextLeaderId]
				{
					return party->promoteLeader(nextLeaderId);
				}));
			}
			members.push_back(makeUserDto(clients[i], i == nextLeader, Party::PartyUserStatus::NotReady, party->getLocalMember().userData));
		}
		tasks.push_back(awaitSettingsConsistency(clients, settings));
		tasks.push_back(awaitMembersConsistency(clients, members));

		return when_all_handle_exceptions(tasks);
	}

	static pplx::task<void> testFindGame(
		const std::vector<std::shared_ptr<Stormancer::IClient>>& clients1,
		const std::vector<std::shared_ptr<Stormancer::IClient>>& clients2,
		std::chrono::seconds findGameTimeout = std::chrono::seconds(10)
	)
	{
		using namespace Stormancer;
		using namespace TestHelpers;

		std::vector<pplx::task<void>> tasks;
		tasks.reserve(clients1.size() + clients2.size());

		std::transform(clients1.begin(), clients1.end(), std::back_inserter(tasks), memberFindGame);
		std::transform(clients2.begin(), clients2.end(), std::back_inserter(tasks), memberFindGame);

		return taskFailAfterTimeout(when_all_handle_exceptions(tasks), "GameFinder request timed out after " + std::to_string(findGameTimeout.count()) + " seconds", findGameTimeout);
	}

	static pplx::task<void> memberFindGame(std::shared_ptr<Stormancer::IClient> client)
	{
		using namespace Stormancer;
		using namespace TestHelpers;

		auto party = getParty(client);
		pplx::task_completion_event<void> gameFoundTce;

		auto sub = party->subscribeOnGameFound([gameFoundTce](GameFinderResponse)
		{
			gameFoundTce.set();
		});
		auto sub2 = party->subscribeOnGameFinderFailure([gameFoundTce](Party::PartyGameFinderFailure failure)
		{
			gameFoundTce.set_exception(std::runtime_error(failure.reason));
		});

		return party->updatePlayerStatus(Party::PartyUserStatus::Ready)
			.then([sub, sub2, gameFoundTce]
		{
			return pplx::create_task(gameFoundTce).then([sub, sub2] {});
		});
	}

	static void assertex(bool condition, const std::string& msg)
	{
		if (!condition)
		{
			throw std::runtime_error(msg.c_str());
		}
	}

	template<typename T>
	static void assertex(bool condition, const std::string& msg, pplx::task_completion_event<T> tce)
	{
		if (!condition)
		{
			tce.set_exception(std::runtime_error(msg.c_str()));
		}
	}

	template<typename Iterator>
	static void handleExceptions(Iterator begin, Iterator end)
	{
		for (auto it = begin; it != end; it++)
		{
			it->then([](auto t)
			{
				try
				{
					t.get();
				}
				catch (...) {}
			});
		}
	}

	template<typename T>
	static pplx::task<void> when_all_handle_exceptions(const std::vector<pplx::task<T>>& tasks)
	{
		return pplx::when_all(tasks.begin(), tasks.end()).then([tasks](pplx::task<void> task)
		{
			try
			{
				task.get();
			}
			catch (...)
			{
				handleExceptions(tasks.begin(), tasks.end());
				throw;
			}
		});
	}
};
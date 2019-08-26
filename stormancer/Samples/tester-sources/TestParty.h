#pragma once
#include "test.h"
#include "Party/Party.hpp"
#include "Authentication/AuthenticationPlugin.h"
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
		testPartyFull(tester);
	}

private:

	static pplx::task<std::shared_ptr<Stormancer::IClient>> makeClient(const Stormancer::Tester& tester)
	{
		using namespace Stormancer;

		auto conf = Configuration::create(tester.endpoint(), tester.account(), tester.application());
		conf->logger = tester.logger();
		conf->addPlugin(new AuthenticationPlugin);
		conf->addPlugin(new GameFinderPlugin);
		conf->addPlugin(new Party::PartyPlugin);

		auto client = IClient::create(conf);
		auto users = client->dependencyResolver().resolve<AuthenticationService>();
		users->getCredentialsCallback = []
		{
			AuthParameters params;
			params.type = "test";
			params.parameters["testkey"] = "testvalue";
			return pplx::task_from_result<AuthParameters>(params);
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

	static void failAfterTimeout(pplx::task_completion_event<void> tce, std::string msg, std::chrono::milliseconds timeout = std::chrono::milliseconds(1000))
	{
		failAfterTimeout(tce, [msg] { return msg; }, timeout);
	}

	static void failAfterTimeout(pplx::task_completion_event<void> tce, std::function<std::string()> msgBuilder, std::chrono::milliseconds timeout = std::chrono::milliseconds(1000))
	{
		auto timeoutCt = Stormancer::timeout(timeout);
		timeoutCt.register_callback([tce, msgBuilder]
		{
			tce.set_exception(std::runtime_error(msgBuilder().c_str()));
		});
	}


	template<typename T>
	static pplx::task<T> taskFailAfterTimeout(pplx::task<T> task, std::string msg, std::chrono::milliseconds timeout = std::chrono::milliseconds(1000))
	{
		return Stormancer::cancel_after_timeout(task, static_cast<unsigned int>(timeout.count()))
			.then([msg](pplx::task<T> t)
		{
			auto status = t.wait();
			if (status == pplx::canceled)
			{
				throw std::runtime_error(msg);
			}
			return t;
		});
	}

	static void testCreatePartyBadRequest(const Stormancer::Tester& tester)
	{
		using namespace Stormancer;

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
			return;
		}
		throw std::runtime_error("createParty should have failed because of bad request");
	}

	static void testPartyFull(const Stormancer::Tester& tester)
	{
		using namespace Stormancer;
		using namespace TestHelpers;

		auto clients = makeClients(tester, 15).get();

		tester.logger()->log(LogLevel::Info, "TestParty", "testCreateParty");
		testCreateParty(clients[0]).get();
		tester.logger()->log(LogLevel::Info, "TestParty", "testCreateParty PASSED");

		tester.logger()->log(LogLevel::Info, "TestParty", "testInvitations");
		testInvitations(clients[0], clients.begin()+1, clients.end(), tester).get();
		tester.logger()->log(LogLevel::Info, "TestParty", "testInvitations PASSED");
	}

	static pplx::task<void> testCreateParty(std::shared_ptr<Stormancer::IClient> client)
	{
		using namespace Stormancer;
		using namespace TestHelpers;

		auto party = client->dependencyResolver().resolve<Party::PartyApi>();
		auto users = client->dependencyResolver().resolve<AuthenticationService>();

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
		std::shared_ptr<Stormancer::AuthenticationService> users,
		const Stormancer::Party::PartyRequestDto& request
	)
	{
		using namespace Stormancer;

		pplx::task_completion_event<void> tce;
		auto sub = party->subscribeOnUpdatedPartySettings([tce, party, request, users](Party::PartySettings)
		{
			assertex(party->getPartySettings().customData == request.CustomData, "CustomData should be the same as in the request", tce);
			assertex(party->getPartySettings().gameFinderName == request.GameFinderName, "gameFinderName should be the same as in the request", tce);
			assertex(party->getPartySettings().leaderId == users->userId(), "I should be the leader ; actual leader value from settings: " + party->getPartySettings().leaderId, tce);
			assertex(party->isLeader(), "PartyApi should also report that I'm the leader", tce);
			tce.set();
		});

		failAfterTimeout(tce, "PartySettings update was not received in time", std::chrono::seconds(5));
		return pplx::create_task(tce).then([sub] {});
	}

	static pplx::task<void> testMembersValidityOnPartyJoined(std::shared_ptr<Stormancer::IClient> client)
	{
		using namespace Stormancer;

		auto party = client->dependencyResolver().resolve<Party::PartyApi>();
		auto users = client->dependencyResolver().resolve<AuthenticationService>();

		Party::PartyUserDto expectedMember{};
		expectedMember.isLeader = true;
		expectedMember.partyUserStatus = Party::PartyUserStatus::NotReady;
		expectedMember.userId = users->userId();

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

		pplx::task_completion_event<void> tce;
		auto party = client->dependencyResolver().resolve<Party::PartyApi>();
		auto userId = client->dependencyResolver().resolve<AuthenticationService>();
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

		auto senderUsers = sender->dependencyResolver().resolve<AuthenticationService>();
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

		auto recipientId = recipient->dependencyResolver().resolve<AuthenticationService>()->userId();
		auto inviteTask = party->invitePlayer(recipientId);

		failAfterTimeout(inviteReceivedTce, "invite was not received on time by user "+recipientId);
		return pplx::create_task(inviteReceivedTce).then([=]
		{
			party2inviteSub->unsubscribe();
			auto invites = party2->getPendingInvitations();
			assertex(invites.size() == 1, "User " + recipientId + " should have only 1 invite");

			recipient->dependencyResolver().resolve<ILogger>()->log(LogLevel::Debug, "testInvitation", "User connecting to party", recipientId);
			return taskFailAfterTimeout(party2->joinParty(invites[0]), "JoinParty did not complete in time for user "+recipientId, std::chrono::seconds(5));
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

	static Stormancer::Party::PartyUserDto makeUserDto(std::shared_ptr<Stormancer::IClient> user, bool isLeader = false)
	{
		using namespace Stormancer;

		Party::PartyUserDto dto = {};
		dto.isLeader = isLeader;
		dto.partyUserStatus = Party::PartyUserStatus::NotReady;
		dto.userId = user->dependencyResolver().resolve<AuthenticationService>()->userId();

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
			auto clientId = (*client)->dependencyResolver().template resolve<AuthenticationService>()->userId();
			tasks.push_back(testInvitation(sender, *client).then([logger, clientId]
			{
				logger->log(LogLevel::Debug, "testInvitations", "Invite complete", clientId);
			}));
			tasks.push_back(awaitMembersConsistency(*client, users, std::chrono::seconds(15)).then([logger, clientId]
			{
				logger->log(LogLevel::Debug, "testInvitations", "MemberConsistency complete recipient", clientId);
			}));
		}
		auto senderId = sender->dependencyResolver().resolve<AuthenticationService>()->userId();
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

	static pplx::task<void> testSettingsUpdate(std::vector<std::shared_ptr<Stormancer::IClient>> clients)
	{
		// TODO
		return pplx::task_from_result();
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
};
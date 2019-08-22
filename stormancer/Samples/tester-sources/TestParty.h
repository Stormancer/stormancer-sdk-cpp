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

		auto clients = makeClients(tester, 2).get();

		tester.logger()->log(LogLevel::Info, "TestParty", "testCreateParty");
		testCreateParty(clients[0]).get();
		tester.logger()->log(LogLevel::Info, "TestParty", "testCreateParty PASSED");

		tester.logger()->log(LogLevel::Info, "TestParty", "testInvitation");
		testInvitation(clients[0], clients[1]).get();
		tester.logger()->log(LogLevel::Info, "TestParty", "testInvitation PASSED");
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
			SelfObservingTask<void> checkMembersTask = testMembersValidityOnPartyJoined(party, users);

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

	static pplx::task<void> testMembersValidityOnPartyJoined(std::shared_ptr<Stormancer::Party::PartyApi> party,
		std::shared_ptr<Stormancer::AuthenticationService> users
	)
	{
		using namespace Stormancer;

		Party::PartyUserDto expectedMember{};
		expectedMember.isLeader = true;
		expectedMember.partyUserStatus = Party::PartyUserStatus::NotReady;
		expectedMember.userId = users->userId();

		return awaitMembersConsistency(party, { expectedMember });
	}

	static pplx::task<void> testMemberCountUpdate(std::vector<std::shared_ptr<Stormancer::Party::PartyApi>> parties, int expectedCount)
	{
		using namespace Stormancer;

		std::vector<Stormancer::Subscription> subs;
		std::vector<pplx::task<void>> tasks;
		for (auto party : parties)
		{
			pplx::task_completion_event<void> tce;
			tasks.push_back(pplx::create_task(tce));
			subs.push_back(party->subscribeOnUpdatedPartyMembers([tce, expectedCount](std::vector<Party::PartyUserDto> members)
			{
				if (members.size() == expectedCount)
				{
					tce.set();
				}
			}));
			failAfterTimeout(tce, "did not get member count update in time", std::chrono::seconds(2));
		}

		return pplx::when_all(tasks.begin(), tasks.end())
			.then([subs, tasks](pplx::task<void> task)
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

	static pplx::task<void> awaitMembersConsistency(std::shared_ptr<Stormancer::Party::PartyApi> party, const std::vector<Stormancer::Party::PartyUserDto>& expectedMembers)
	{
		using namespace Stormancer;

		pplx::task_completion_event<void> tce;
		auto sub = party->subscribeOnUpdatedPartyMembers([tce, expectedMembers](std::vector<Party::PartyUserDto> members)
		{
			if (membersAreEqual(members, expectedMembers))
			{
				tce.set();
			}
		});

		if (membersAreEqual(party->getPartyMembers(), expectedMembers))
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
			ss << membersToString(party->getPartyMembers());
			return ss.str();
		}, std::chrono::seconds(3));

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

		SelfObservingTask<void> inviteTask(party->invitePlayer(recipient->dependencyResolver().resolve<AuthenticationService>()->userId()));

		failAfterTimeout(inviteReceivedTce, "invite was not received on time");
		return pplx::create_task(inviteReceivedTce).then([=]
		{
			party2inviteSub->unsubscribe();
			auto invites = party2->getPendingInvitations();
			assertex(invites.size() == 1, "User 2 should have only 1 invite");

			return party2->joinParty(invites[0]);
		})
			.then([=]
		{
			return taskFailAfterTimeout(*inviteTask, "Invitation task should have completed after invitee joined the party", std::chrono::seconds(5));
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
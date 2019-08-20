#pragma once
#include "test.h"
#include "Party/Party.hpp"
#include "Authentication/AuthenticationPlugin.h"
#include "GameFinder/GameFinderPlugin.h"
#include "stormancer/Utilities/TaskUtilities.h"
#include "TestHelpers.h"

class TestParty
{
public:
	static void runTests(const Stormancer::Tester& tester)
	{
		testCreatePartyBadRequest(tester);
		testCreateParty(tester);
	}

private:

	class AutoCanceler
	{
	public:
		pplx::cancellation_token getToken() { return cts.get_token(); }

		~AutoCanceler() { cts.cancel(); }
	private:
		pplx::cancellation_token_source cts;
	};

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

	static void failAfterTimeout(pplx::task_completion_event<void> tce, std::string msg, pplx::cancellation_token ct, std::chrono::milliseconds timeout = std::chrono::milliseconds(1000))
	{
		auto timeoutCt = Stormancer::timeout(timeout);
		auto registration = timeoutCt.register_callback([tce, msg]
		{
			tce.set_exception(std::runtime_error(msg));
		});
		ct.register_callback([registration, timeoutCt]
		{
			timeoutCt.deregister_callback(registration);
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

	static void testCreateParty(const Stormancer::Tester& tester)
	{
		using namespace Stormancer;
		using namespace TestHelpers;

		auto clients = makeClients(tester, 2).get();
		auto client = clients[0];
		auto party = client->dependencyResolver().resolve<Party::PartyApi>();
		auto users = client->dependencyResolver().resolve<AuthenticationService>();

		// If the tester throws, this will cancel any pending task that could also throw
		// (avoids unobserved exceptions with concurrent testing tasks)
		AutoCanceler canceler;

		Party::PartyRequestDto request = {};
		request.platformSessionId = ""; // This will make a guid
		request.GameFinderName = "testGameFinder";
		request.CustomData = "customData";

		pplx::task_completion_event<void> joinedSubTce;
		auto subJoined = party->subscribeOnJoinedParty([joinedSubTce]
		{
			joinedSubTce.set();
		});
		SelfObservingTask<void> checkSettingsTask(testSettingsValidityOnPartyJoined(party, users, request, canceler.getToken()));
		SelfObservingTask<void> checkMembersTask(testMembersValidityOnPartyJoined(party, users, canceler.getToken()));

		party->createParty(request).get();

		assertex(party->isInParty(), "I should be in a party after a successful createParty");
		assertex(party->getPendingInvitations().empty(), "Pending invitations should be empty");
		
		failAfterTimeout(joinedSubTce, "OnJoinedParty subscription was not triggered in time", canceler.getToken());
		SelfObservingTask<void> joinedSubTask(pplx::create_task(joinedSubTce));
		auto creationCheckTasks = { joinedSubTask.task, checkSettingsTask.task, checkMembersTask.task };
		pplx::when_all(creationCheckTasks.begin(), creationCheckTasks.end()).get();

		auto client2 = clients[1];
		auto party2 = client2->dependencyResolver().resolve<Party::PartyApi>();

		auto senderId = users->userId();
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

		SelfObservingTask<void> inviteTask(party->invitePlayer(client2->dependencyResolver().resolve<AuthenticationService>()->userId()));

		failAfterTimeout(inviteReceivedTce, "invite was not received on time", canceler.getToken());
		pplx::create_task(inviteReceivedTce).get();

		auto invites = party2->getPendingInvitations();
		assertex(invites.size() == 1, "User 2 should have only 1 invite");

		SelfObservingTask<void> countUpdateTest = testMemberCountUpdate({ party, party2 }, 2, canceler.getToken());
		
		party2->joinParty(invites[0]).get();
		tester.logger()->log(LogLevel::Info, "TestParty", "Waiting for invite completion...");
		taskFailAfterTimeout(inviteTask.task, "Invitation task should have completed after invitee joined the party", std::chrono::seconds(5)).get();
		countUpdateTest.task.get();
	}

	static pplx::task<void> testSettingsValidityOnPartyJoined(std::shared_ptr<Stormancer::Party::PartyApi> party,
		std::shared_ptr<Stormancer::AuthenticationService> users,
		const Stormancer::Party::PartyRequestDto& request,
		pplx::cancellation_token ct)
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

		failAfterTimeout(tce, "PartySettings update was not received in time", ct, std::chrono::seconds(5));
		return pplx::create_task(tce).then([sub] {});
	}

	static pplx::task<void> testMembersValidityOnPartyJoined(std::shared_ptr<Stormancer::Party::PartyApi> party,
		std::shared_ptr<Stormancer::AuthenticationService> users,
		pplx::cancellation_token ct)
	{
		using namespace Stormancer;

		pplx::task_completion_event<void> tce;
		auto sub = party->subscribeOnUpdatedPartyMembers([tce, party, users](std::vector<Party::PartyUserDto>)
		{
			auto members = party->getPartyMembers();
			assertex(members.size() == 1, "there should be exactly 1 member ; currently there is " + std::to_string(members.size()), tce);
			assertex(members[0].userId == users->userId(), "the member's Id should be my Id ; instead, it is " + members[0].userId, tce);
			assertex(members[0].isLeader == true, "I should be the leader", tce);
			assertex(members[0].partyUserStatus == Party::PartyUserStatus::NotReady, "My status should be NotReady", tce);
			tce.set();
		});

		failAfterTimeout(tce, "PartyMembers update was not received in time", ct, std::chrono::seconds(5));
		return pplx::create_task(tce).then([sub] {});
	}

	static pplx::task<void> testMemberCountUpdate(std::vector<std::shared_ptr<Stormancer::Party::PartyApi>> parties, int expectedCount,
		pplx::cancellation_token ct)
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
				else
				{
					tce.set_exception(std::runtime_error("Bad member count: got " + std::to_string(members.size()) + ", should be " + std::to_string(expectedCount)));
				}
			}));
			failAfterTimeout(tce, "did not get member count update in time", ct, std::chrono::seconds(2));
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
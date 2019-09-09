#pragma once

#include "Users/Users.hpp"
#include "Friends/Friends.hpp"
#include "stormancer/StormancerTypes.h"
#include "stormancer/IPlugin.h"
#include "cpprest/asyncrt_utils.h"
#include "steam/steam_api.h"
#include "stormancer/Utilities/PointerUtilities.h"
#include "stormancer/Configuration.h"
#include "stormancer/IScheduler.h"

// https://partner.steamgames.com/doc/sdk/api
// https://partner.steamgames.com/doc/features/auth#client_to_backend_webapi
// https://partner.steamgames.com/doc/api/ISteamUser#GetAuthSessionTicket

namespace Stormancer
{
	namespace Steam
	{
		namespace Details
		{
			class SteamService
			{
			public:

				SteamService(std::shared_ptr<Configuration> config, std::shared_ptr<IScheduler> scheduler)
				{
					_initAndRunSteam = (config->additionalParameters.at("steam.initAndRunSteam") == "true");
					_forcedTicket = config->additionalParameters.at("steam.forcedTicket");

					if (_initAndRunSteam)
					{
						scheduler->schedulePeriodic(100, [actionDispatcher = config->actionDispatcher]()
						{
							pplx::create_task([]()
							{
								SteamAPI_RunCallbacks();
							}, actionDispatcher);
						}, _cts.get_token());
					}
				}

				~SteamService()
				{
					_cts.cancel();
				}

				bool getInitAndRunSteam()
				{
					return _initAndRunSteam;
				}

				std::string getForcedTicket()
				{
					return _forcedTicket;
				}

			private:

				bool _initAndRunSteam = false;
				std::string _forcedTicket;
				pplx::cancellation_token_source _cts;
			};
		}

		class SteamAuthenticationEventHandler : public std::enable_shared_from_this<SteamAuthenticationEventHandler>, public Users::IAuthenticationEventHandler
		{
		public:

			SteamAuthenticationEventHandler(std::shared_ptr<Details::SteamService> steamService)
			{
				_initAndRunSteam = steamService->getInitAndRunSteam();
				_forcedTicket = steamService->getForcedTicket();
			}

			pplx::task<void> retrieveCredentials(const Users::CredientialsContext& context)
			{
				std::lock_guard<std::recursive_mutex> lg(_mutex);
				
				_tce = std::make_shared<pplx::task_completion_event<void>>();

				if (!SteamAPI_IsSteamRunning())
				{
					throw std::runtime_error("Steam is not running");
				}

				auto ct = pplx::cancellation_token::none();

				if (_initAndRunSteam)
				{
					if (!SteamAPI_Init())
					{
						throw std::runtime_error("Steam init failed");
					}

					ct = timeout(10s);
					auto wSteamAuthEventHandler = STORM_WEAK_FROM_THIS();
					ct.register_callback([wSteamAuthEventHandler]()
					{
						if (auto steamAuthEventHandler = wSteamAuthEventHandler.lock())
						{
							std::lock_guard<std::recursive_mutex> lg(steamAuthEventHandler->_mutex);
							steamAuthEventHandler->_tce.reset();
						}
					});
				}

				std::string steamTicketHex;

				std::shared_ptr<std::vector<byte>> steamTicket;

				if (_forcedTicket.empty())
				{
					auto steamUser = SteamUser();
					if (steamUser == nullptr)
					{
						throw std::runtime_error("SteamUser() returned null");
					}

					steamTicket = std::make_shared<std::vector<byte>>(1024);

					auto pcbTicket = std::make_shared<uint32>(0);
					auto hAuthTicket = steamUser->GetAuthSessionTicket(steamTicket->data(), (int)steamTicket->size(), pcbTicket.get());

					if (hAuthTicket == k_HAuthTicketInvalid)
					{
						throw std::runtime_error("Steam : invalid user authentication ticket");
					}
				}
				else
				{
					steamTicket = std::make_shared<std::vector<byte>>(_forcedTicket.data(), _forcedTicket.data() + _forcedTicket.size());
					_tce->set();
				}

				return pplx::create_task(*_tce, ct)
					.then([context, steamTicket]()
				{
					std::stringstream ss;
					ss << std::uppercase << std::hex << std::setfill('0');
					for (auto b : *steamTicket)
					{
						ss << std::setw(2) << static_cast<unsigned>(b);
					}
					auto steamTicketHex = ss.str();

					context.authParameters->type = "steam";
					context.authParameters->parameters = {
						{ "provider", "steam" },
						{ "ticket", steamTicketHex }
					};
				});
			}

		private:

			STEAM_CALLBACK(SteamAuthenticationEventHandler, onAuthSessionTicket, GetAuthSessionTicketResponse_t);

			std::recursive_mutex _mutex;
			bool _initAndRunSteam = false;
			std::string _forcedTicket;
			std::shared_ptr<pplx::task_completion_event<void>> _tce;
		};

		class SteamPlugin : public IPlugin
		{
			void registerClientDependencies(ContainerBuilder& builder) override
			{
				builder.registerDependency<Details::SteamService, Configuration, IScheduler>().singleInstance();
				builder.registerDependency<SteamAuthenticationEventHandler, Details::SteamService>().as<Users::IAuthenticationEventHandler>();
			}

			void clientCreated(std::shared_ptr<IClient> client)
			{
				client->dependencyResolver().resolve<Details::SteamService>(); // Create the single instance steam service
			}
		};

		void SteamAuthenticationEventHandler::onAuthSessionTicket(GetAuthSessionTicketResponse_t* pCallback)
		{
			std::lock_guard<std::recursive_mutex> lg(_mutex);

			if (pCallback->m_eResult != EResult::k_EResultOK)
			{
				_tce->set_exception(std::runtime_error("Steam : failed to get auth session ticket (EResult = " + std::to_string((int)pCallback->m_eResult) + ")"));
			}

			if (pCallback->m_hAuthTicket == k_HAuthTicketInvalid)
			{
				_tce->set_exception(std::runtime_error("Steam : invalid user authentication ticket"));
			}

			if (_tce)
			{
				_tce->set();
			}
			else
			{
				throw std::runtime_error("Steam auth callback: tce not available");
			}
		}
	}
}

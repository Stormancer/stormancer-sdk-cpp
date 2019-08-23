#pragma once

#include "Users/Users.hpp"
#include "Friends/Friends.hpp"
#include "stormancer/StormancerTypes.h"
#include "stormancer/IPlugin.h"
#include "cpprest/asyncrt_utils.h"
#include "steam/steam_api.h"
#include "stormancer/Utilities/PointerUtilities.h"

namespace Stormancer
{
	namespace Steam
	{
		class SteamAuthenticationEventHandler : public std::enable_shared_from_this<SteamAuthenticationEventHandler>, public Users::IAuthenticationEventHandler
		{
		public:

			SteamAuthenticationEventHandler() = default;

			SteamAuthenticationEventHandler(std::string forcedTicket)
				: _forceTicket(true)
				, _forcedTicket(forcedTicket)
			{
			}

			pplx::task<void> retrieveCredentials(const Users::CredientialsContext& context)
			{
				// https://partner.steamgames.com/doc/sdk/api
				// https://partner.steamgames.com/doc/features/auth#client_to_backend_webapi
				// https://partner.steamgames.com/doc/api/ISteamUser#GetAuthSessionTicket

				if (!SteamAPI_IsSteamRunning())
				{
					throw std::runtime_error("Steam is not running");
				}

				if (!SteamAPI_Init())
				{
					throw std::runtime_error("Steam init failed");
				}

				auto steamUser = SteamUser();
				if (steamUser == nullptr)
				{
					throw std::runtime_error("SteamUser() returned null");
				}

				auto steamTicket = std::make_shared<std::vector<byte>>(1024);
				auto pcbTicket = std::make_shared<uint32>(0);
				auto hAuthTicket = steamUser->GetAuthSessionTicket(steamTicket->data(), (int)steamTicket->size(), pcbTicket.get());

				if (hAuthTicket == k_HAuthTicketInvalid)
				{
					throw std::runtime_error("Steam : invalid user authentication ticket");
				}

				auto forceTicket = _forceTicket;
				auto forcedTicket = _forcedTicket;

				auto ct = timeout(10s);
				auto wSteamAuthEventHandler = STORM_WEAK_FROM_THIS();
				ct.register_callback([wSteamAuthEventHandler]()
				{
					if (auto steamAuthEventHandler = wSteamAuthEventHandler.lock())
					{
						steamAuthEventHandler->_tce.reset();
					}
				});

				return pplx::create_task(*_tce, ct)
					.then([context, steamTicket, forceTicket, forcedTicket]()
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
						{ "ticket", forceTicket ? forcedTicket : steamTicketHex }
					};
				});
			}

		private:

			STEAM_CALLBACK(SteamAuthenticationEventHandler, onAuthSessionTicket, GetAuthSessionTicketResponse_t);

			bool _forceTicket = false;
			std::string _forcedTicket;
			std::shared_ptr<pplx::task_completion_event<void>> _tce = std::make_shared<pplx::task_completion_event<void>>();
		};

		class SteamPlugin : public IPlugin
		{
			void registerClientDependencies(ContainerBuilder& builder) override
			{
				builder.registerDependency<SteamAuthenticationEventHandler>().as<Users::IAuthenticationEventHandler>();
			}
		};

		void SteamAuthenticationEventHandler::onAuthSessionTicket(GetAuthSessionTicketResponse_t* pCallback)
		{
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

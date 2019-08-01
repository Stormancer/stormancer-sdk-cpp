#pragma once

#include "Users/Users.hpp"
#include "stormancer/StormancerTypes.h"
#include "stormancer/IPlugin.h"
#include "cpprest/asyncrt_utils.h"

namespace Stormancer
{
	namespace Steam
	{
		class SteamAuthenticationEventHandler : public Users::IAuthenticationEventHandler
		{
		public:

			SteamAuthenticationEventHandler() = default;

			SteamAuthenticationEventHandler(std::string forcedToken)
				: _forceToken(true)
				, _forcedToken(forcedToken)
			{
			}

			pplx::task<void> retrieveCredentials(const Users::CredientialsContext& context)
			{
				// https://partner.steamgames.com/doc/features/auth#client_to_backend_webapi
				// https://partner.steamgames.com/doc/api/ISteamUser#GetAuthSessionTicket
				std::vector<byte> steamTicket(1024);
				uint32 pcbTicket;
				auto HAuthTicket = ISteamUser::GetAuthSessionTicket(steamTicket.data(), steamTicket.size(), &pcbTicket);
				auto steamTicketBase64 = utility::conversions::to_base64(steamTicket);

				context.authParameters->type = "steam";
				context.authParameters->parameters = {
					{ "provider", "steam" },
					{ "ticket", _forceToken ? _forcedToken : steamTicketBase64 }
				};

				return pplx::task_from_result();
			}

		private:

			bool _forceToken = false;
			std::string _forcedToken;
		};

		class SteamPlugin : public IPlugin
		{
			void registerClientDependencies(ContainerBuilder& builder) override
			{
				builder.registerDependency<SteamAuthenticationEventHandler>().as<Users::IAuthenticationEventHandler>();
			}
		};
	}
}

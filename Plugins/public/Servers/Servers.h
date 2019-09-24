#pragma once
#include "Servers/ServersModels.h"


namespace Stormancer
{
	class Servers
	{
	public:
		virtual ~Servers() {}
	
		/// <summary>
		///  Get list of accessible environments for the connected client
		/// </summary>
		/// <returns>Description of all accessible server</returns>
		virtual pplx::task<std::vector<ServerDescription>> GetServersDescription() = 0;
		
		/// <summary>
		/// Select the server environment to connect to form for this session.
		/// </summary>
		/// <param name="serverId">Target server Id</param>
		/// <returns></returns>
		virtual pplx::task<void> SelectServer(const std::string serverId) = 0;
	};
}
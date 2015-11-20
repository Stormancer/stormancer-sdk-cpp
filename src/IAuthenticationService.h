#pragma once
#include "headers.h"

namespace Stormancer
{
	class IAuthenticationService
	{
	public:
		virtual const char* authenticationSceneName() = 0;
		virtual void setAuthenticationSceneName(const char* name) = 0;

		virtual const char* createUserRoute() = 0;
		virtual void setCreateUserRoute(const char* name) = 0;

		virtual const char* loginRoute() = 0;
		virtual void setLoginRoute(const char* name) = 0;

		virtual pplx::task<Scene*> login(const char* pseudo, const char* password) = 0;
		virtual pplx::task<Scene*> login(const stringMap* authenticationContext) = 0;
		virtual pplx::task<Scene*> steamLogin(const char* steamTicket) = 0;

		virtual pplx::task<Scene*> getAuthenticationScene() = 0;

		virtual pplx::task<void> logout() = 0;
	};
};

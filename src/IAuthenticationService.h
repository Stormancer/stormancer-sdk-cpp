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

		virtual pplx::task<Result<Scene*>*> login(const char* pseudo, const char* password) = 0;
		virtual pplx::task<Result<Scene*>*> login(const stringMap* authenticationContext) = 0;

		virtual pplx::task<Result<Scene*>*> steamLogin(const char* steamTicket) = 0;

		virtual pplx::task<Scene*> getAuthenticationScene() = 0;
		
		/// <summary>
		/// Gets a scene token from the server
		/// </summary>
		/// <param name="sceneId">Id of the scene to access.</param>
		/// <returns>Scene object to configure then connect to.</returns>
		virtual pplx::task<Result<Scene*>*> getPrivateScene(const char* sceneId) = 0;

		virtual pplx::task<Result<>*> logout() = 0;

		virtual const char* userId() = 0;
	};
};

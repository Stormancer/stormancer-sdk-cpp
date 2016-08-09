#pragma once
#include <stormancer.h>

namespace Stormancer
{
	enum class GameConnectionState
	{
		Disconnected = 0,
		Connecting = 1,
		Authenticated = 2,
		Disconnecting = 3,
		Authenticating = 4,

	};
	
	class IAuthenticationService
	{
	public:
		virtual const char* authenticationSceneName() = 0;
		virtual void setAuthenticationSceneName(const char* name) = 0;

		virtual const char* createUserRoute() = 0;
		virtual void setCreateUserRoute(const char* name) = 0;

		virtual const char* loginRoute() = 0;
		virtual void setLoginRoute(const char* name) = 0;

		virtual pplx::task<Result<>*> createAccount(std::string login, std::string password, std::string email, std::string key,std::string pseudo)=0;
		virtual pplx::task<Result<Scene*>*> login(std::string pseudo, std::string password) = 0;
		virtual pplx::task<Result<Scene*>*> login(const stringMap authenticationContext) = 0;

		virtual pplx::task<Result<Scene*>*> steamLogin(std::string steamTicket) = 0;

		virtual pplx::task<Scene*> getAuthenticationScene() = 0;
		
		virtual pplx::task<Result<Scene*>*> getPrivateScene(std::string sceneId) = 0;

		virtual pplx::task<std::shared_ptr<Result<void>>> requestPasswordChange(std::string email) = 0;
		virtual pplx::task<std::shared_ptr<Result<void>>> changePassword(std::string email, std::string code, std::string newPassword) = 0;
		/// <summary>
		/// Gets a scene token from the server
		/// </summary>
		/// <param name="sceneId">Id of the scene to access.</param>
		/// <returns>Scene object to configure then connect to.</returns>
		//virtual pplx::task<Result<Scene*>*> getPrivateScene(const char* sceneId) = 0;

		virtual pplx::task<Result<>*> logout() = 0;

		virtual const char* userId() = 0;
		virtual const char* GetUsername() = 0;

		virtual GameConnectionState connectionState() const= 0;
		virtual Action<GameConnectionState>& connectionStateChangedAction() = 0;
		virtual Action<GameConnectionState>::TIterator onConnectionStateChanged(std::function<void(GameConnectionState)> callback) = 0;
	};

	
};

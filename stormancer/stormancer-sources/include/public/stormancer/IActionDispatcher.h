#pragma once

#include "stormancer/BuildConfig.h"

#include <functional>
#include <queue>
#include "stormancer/Tasks.h"

namespace Stormancer
{
	//Dispatches client actions to the appropriate thread.
	//The default dispatcher execute client actions on the current network engine thread.
	//Developers should use an instance of the MainThreadActionDispatcher to dispatch client actions to the game main thread.
	class IActionDispatcher : public pplx::scheduler_interface
	{
	public:

#pragma region public_methods

		virtual void post(const std::function<void(void)>& action) = 0;
		virtual void start() = 0;
		virtual pplx::task<void> stop() = 0;
		virtual void schedule(pplx::TaskProc_t, void *) = 0;
		virtual bool isRunning() = 0;
		virtual ~IActionDispatcher();
#pragma endregion
	};

	class SameThreadActionDispatcher : public IActionDispatcher
	{
	public:

#pragma region public_methods

		virtual void post(const std::function<void(void)>& action) override;
		virtual void start() override;
		virtual pplx::task<void> stop() override;
		virtual bool isRunning() override;
		virtual ~SameThreadActionDispatcher();
#pragma endregion

	private:

#pragma region private_methods

		virtual void schedule(pplx::TaskProc_t, void *) override;

#pragma endregion

#pragma region private_members

		bool _isRunning;

#pragma endregion
	};

	class MainThreadActionDispatcher :public IActionDispatcher
	{
	public:

#pragma region public_methods

		virtual void post(const std::function<void(void)>& action) override;
		virtual void start() override;
		virtual pplx::task<void> stop() override;
		virtual bool isRunning() override;
		//Runs the actions in the current thread.
		//@maxDuration: Max duration the update method should run (it can take more time, but as soon as  maxDuration is exceeded, update will return as soon as the
		//current executing action returns.)
		void update(const std::chrono::milliseconds maxDuration);

		virtual ~MainThreadActionDispatcher();
#pragma endregion

	private:

#pragma region private_methods

		virtual void schedule(pplx::TaskProc_t, void *) override;

#pragma endregion

#pragma region private_members

		bool _isRunning;
		bool _stopRequested;
		std::queue<std::function<void(void)>> _actions;
		std::mutex _mutex;
		pplx::task_completion_event<void> _stopTce;

#pragma endregion
	};
}
